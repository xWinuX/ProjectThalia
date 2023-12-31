#include "SplitEngine/Application.hpp"
#include "SplitEngine/AssetDatabase.hpp"
#include "SplitEngine/Debug/Performance.hpp"
#include "SplitEngine/ECS/System.hpp"
#include "SplitEngine/Rendering/Sprite.hpp"
#include "SplitEngine/Rendering/Texture2D.hpp"
#include "SplitEngine/Tools/ImagePacker.hpp"
#include "SplitEngine/Tools/ImageSlicer.hpp"
#include "imgui.h"

#include <execution>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace SplitEngine;


// enum classes are especially good for asset keys, since they don't interfere with each others member names
enum class Shader
{
	Sprite,
};

enum class Material
{
	Sprite,
};

enum class Sprite
{
	Salice,
	Floppa,
};

/**
 * Components should only contain data
 * The easier to copy the better the performance!
 */
struct TransformComponent
{
		glm::vec3 Position;
		float     Rotation;
};

struct SpriteComponent
{
		AssetHandle<Rendering::Sprite> Sprite;
		float                          AnimationSpeed = 0;
		float                          CurrentFrame   = 0;
};

struct PhysicsComponent
{
		bool      HasGravity;
		glm::vec3 Velocity;
};

// Systems can be as complex as you want and can contain a lot of state
class SpriteRenderSystem : public ECS::System<TransformComponent, SpriteComponent>
{
	public:
		SpriteRenderSystem(AssetHandle<Rendering::Material> material, Tools::ImagePacker::PackingData& packingData) :
			_material(material),
			ECS::System<TransformComponent, SpriteComponent>()
		{
			_cameraUBO    = _material->GetDescriptorSetAllocation().ShaderBuffers[0].GetMappedData<CameraUBO>();
			_objectBuffer = _material->GetDescriptorSetAllocation().ShaderBuffers[1].GetMappedData<ObjectBuffer>();
			_textureStore = _material->GetDescriptorSetAllocation().ShaderBuffers[2].GetMappedData<TextureStore>();

			for (int i = 0; i < packingData.PackingInfos.size(); ++i)
			{
				Tools::ImagePacker::PackingInfo packingInfo        = packingData.PackingInfos[i];
				_textureStore->Textures[i].PageIndexAndAspectRatio = {packingInfo.PageIndex, packingInfo.AspectRatio};
				_textureStore->Textures[i].UVs[0]                  = {packingInfo.UVTopLeft, 0.0f, 0.0f};
				_textureStore->Textures[i].UVs[1]                  = {packingInfo.UVTopRight, 0.0f, 0.0f};
				_textureStore->Textures[i].UVs[2]                  = {packingInfo.UVBottomLeft, 0.0f, 0.0f};
				_textureStore->Textures[i].UVs[3]                  = {packingInfo.UVBottomRight, 0.0f, 0.0f};
			}

			for (const auto& pageImage : packingData.PageImages)
			{
				_texturePages.push_back(std::make_unique<Rendering::Texture2D, Rendering::Texture2D::CreateInfo>({pageImage, {}}));
			}

			_material->SetTextures(0, 0, _texturePages);

			const std::vector<uint32_t> quad        = {0, 1, 2, 3};
			const std::vector<uint16_t> quadIndices = {0, 1, 2, 2, 1, 3};

			std::vector<uint32_t> vertices;
			vertices.reserve(10240 * 4);
			std::vector<uint16_t> indices;
			indices.reserve(10240 * 6);
			for (size_t i = 0; i < 10240; i++)
			{
				for (const uint32_t& vertex : quad) { vertices.push_back(vertex); }

				for (uint16_t index : quadIndices) { indices.push_back(index + (i * 6)); }
			}

			_model = std::make_unique<Rendering::Model, Rendering::Model::CreateInfo>({reinterpret_cast<std::vector<std::byte>&>(vertices), indices});
		}

		void Execute(TransformComponent* transformComponents, SpriteComponent* spriteComponents, std::vector<uint64_t>& entities, ECS::Context& context) final
		{
			size_t numEntities = entities.size();

			memcpy(_objectBuffer->positions.data(), transformComponents, numEntities * sizeof(glm::vec4));

			_indexes = std::ranges::iota_view((size_t) 0, entities.size());
			std::for_each(std::execution::par, _indexes.begin(), _indexes.end(), [this, spriteComponents, context](size_t i) {
				SpriteComponent&   spriteAnimatorComponent = spriteComponents[i];
				float&             currentFrame            = spriteAnimatorComponent.CurrentFrame;
				Rendering::Sprite* sprite                  = spriteAnimatorComponent.Sprite.Get();

				currentFrame = FastFmod(currentFrame + spriteAnimatorComponent.AnimationSpeed * context.DeltaTime,
										static_cast<float>(sprite->GetNumSubSprites()));

				_objectBuffer->textureIDs[i] = sprite->GetTextureID(static_cast<uint32_t>(currentFrame));
			});


			vk::CommandBuffer commandBuffer = context.RenderingContext->GetCommandBuffer();

			_cameraUBO->view = glm::lookAt(glm::vec3(0.0f, 0.0f, -200.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			_cameraUBO->proj = glm::perspective(glm::radians(45.0f),
												static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().width) /
														-static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().height),
												0.1f,
												1000.0f);

			_cameraUBO->viewProj = _cameraUBO->proj * _cameraUBO->view;

			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _material->GetShader()->GetPipeline().GetVkPipeline());

			_material->Update();

			vk::Buffer     vertexBuffers[] = {_model->GetModelBuffer().GetVkBuffer()};
			vk::DeviceSize offsets[]       = {0};

			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
			commandBuffer.bindIndexBuffer(_model->GetModelBuffer().GetVkBuffer(), _model->GetModelBuffer().GetSizeInBytes(0), vk::IndexType::eUint16);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
											 _material->GetShader()->GetPipeline().GetLayout(),
											 0,
											 1,
											 &_material->GetDescriptorSetAllocation().DescriptorSet,
											 0,
											 nullptr);

			commandBuffer.drawIndexed(_model->GetModelBuffer().GetBufferElementNum(1), std::max(1u, static_cast<uint32_t>(numEntities) / 10240u), 0, 0, 0);
		}


	private:
		static inline float FastFmod(float a, float b) { return ((a) - ((int) ((a) / (b))) * (b)); }

		struct CameraUBO
		{
				glm::mat4 view;
				glm::mat4 proj;
				glm::mat4 viewProj;
		};

		struct TextureData
		{
				glm::vec2 PageIndexAndAspectRatio {};
				alignas(16) glm::vec4 UVs[4];
		};

		struct TextureStore
		{
				std::array<TextureData, 10240> Textures;
		};

		struct ObjectBuffer
		{
				std::array<glm::vec4, 10'240'000> positions;
				std::array<uint32_t, 10'240'000>  textureIDs;
		};

		ObjectBuffer* _objectBuffer;
		CameraUBO*    _cameraUBO;
		TextureStore* _textureStore;

		AssetHandle<Rendering::Material> _material;

		std::vector<std::unique_ptr<Rendering::Texture2D>> _texturePages;

		std::unique_ptr<Rendering::Model> _model;

		std::ranges::iota_view<size_t, size_t> _indexes;
};

// Example of a simple physics system that changes an entities transform component based on a physicscomponents velocity
class PhysicsSystem : public ECS::System<TransformComponent, PhysicsComponent>
{
	public:
		PhysicsSystem() :
			ECS::System<TransformComponent, PhysicsComponent>()
		{}

		/**
		 * This execute function can be override if you need more control over how archetypes are handeled
		 * You could for example handle the each archtype in a different thread.
		 * If you don't override this function it will call the other execute function with all required parameters thanks to template magic!
		 */
		void Execute(std::vector<ECS::ArchetypeBase*>& archetypes, SplitEngine::ECS::Context& context) override
		{
			size_t numEntities = 0;
			for (const auto& archetype : archetypes)
			{
				TransformComponent* transformComponents = reinterpret_cast<TransformComponent*>(archetype->GetComponents<TransformComponent>().data());
				PhysicsComponent*   physicsComponents   = reinterpret_cast<PhysicsComponent*>(archetype->GetComponents<PhysicsComponent>().data());

				Execute(transformComponents, physicsComponents, archetype->Entities, context);

				numEntities += archetype->Entities.size();
			}

			ImGui::Text("Num Physics Entities: %llu", numEntities);
		}

		/**
		 * The execute function contains a pointer to each component array used by the system
		 * This function can run multiple times and is called for each archetype contains all components needed by this system
		 */
		void Execute(TransformComponent* transformComponents, PhysicsComponent* physicsComponents, std::vector<uint64_t>& entities, ECS::Context& context) final
		{
			_indexes = std::ranges::iota_view((size_t) 0, entities.size());
			std::for_each(std::execution::par, _indexes.begin(), _indexes.end(), [transformComponents, physicsComponents, context](size_t i) {
				transformComponents[i].Position += physicsComponents[i].Velocity * context.DeltaTime;
			});
		}

		std::ranges::iota_view<size_t, size_t> _indexes;
};

int main()
{
	Application application = Application({});
	application.Initialize();

	AssetDatabase& assetDatabase = application.GetAssetDatabase();

	/**
	 * You can create an Asset with every type that has a public struct named CreateInfo!
	 * That means you can also create custom assets
	 * The key can be anything as long as it can be cast into an int, so no magic strings!
	 */
	AssetHandle<Rendering::Shader>   shader   = assetDatabase.CreateAsset<Rendering::Shader>(Shader::Sprite, {"res/shaders/debug"});
	AssetHandle<Rendering::Material> material = assetDatabase.CreateAsset<Rendering::Material>(Material::Sprite, {shader});

	// Create texture page and sprite assets
	Tools::ImagePacker texturePacker = Tools::ImagePacker();

	uint64_t floppaPackerID = texturePacker.AddImage("res/textures/floppa.png");
	uint64_t salicePackerID = texturePacker.AddRelatedImages(Tools::ImageSlicer::Slice("res/textures/salice_move_front.png", {5}));

	Tools::ImagePacker::PackingData packingData = texturePacker.Pack(2048);

	AssetHandle<Rendering::Sprite> floppaSprite = assetDatabase.CreateAsset<Rendering::Sprite>(Sprite::Floppa, {floppaPackerID, packingData});
	AssetHandle<Rendering::Sprite> saliceSprite = assetDatabase.CreateAsset<Rendering::Sprite>(Sprite::Salice, {salicePackerID, packingData});

	// Setup ECS
	ECS::Registry& ecs = application.GetECSRegistry();

	// Each component that is used needs to be registered
	ecs.RegisterComponent<TransformComponent>();
	ecs.RegisterComponent<SpriteComponent>();
	ecs.RegisterComponent<PhysicsComponent>();

	/**
	 * 	Each system can either be registered as a gameplay system or as a render system
	 * 	Gameplay systems execute before rendering systems and don't have any special context
	 * 	Render systems execute after gameplay systems and run in a vulkan context so draw calls/binds etc... can be made
	 * 	Systems can be registered multiple times and parameters get forwarded to the systems constructor
	 */
	ecs.RegisterGameplaySystem<PhysicsSystem>();
	ecs.RegisterRenderSystem<SpriteRenderSystem>(material, packingData);

	// Benchmark time taken until BENCHMARK_END
	BENCHMARK_BEGIN

	// Create entities
	for (int i = 0; i < 1'024'000; ++i)
	{
		ecs.CreateEntity<TransformComponent, PhysicsComponent, SpriteComponent>({glm::ballRand(100.0f), 0.0f},
																				{true, glm::ballRand(1.0f)},
																				{saliceSprite, 6.0f, 0});
	}

	BENCHMARK_END("Entity Creation")

	// Run Game
	application.Run();

	return 0;
}