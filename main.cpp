#include "SplitEngine/Application.hpp"
#include "SplitEngine/AssetDatabase.hpp"
#include "SplitEngine/Debug/Performance.hpp"
#include "SplitEngine/ECS/System.hpp"
#include "SplitEngine/Rendering/Texture2D.hpp"
#include "imgui.h"

#include <execution>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace SplitEngine;

enum class Texture
{
	Floppa,
	EvilFloppa
};

enum class Shader
{
	Sprite,
};

enum class Material
{
	Sprite,
};

enum class Model
{
	SpriteQuads,
};

struct TransformComponent
{
	public:
		glm::vec3 Position;
};

struct SpriteComponent
{
	public:
		uint32_t textureID;
};

struct PhysicsComponent
{
	public:
		bool      HasGravity;
		glm::vec3 Velocity;
};

class SpriteRenderSystem : public ECS::System<TransformComponent, SpriteComponent>
{
	public:
		explicit SpriteRenderSystem(AssetHandle<Rendering::Material> material) :
			_material(material),
			ECS::System<TransformComponent, SpriteComponent>()
		{
			_cameraUBO    = _material->GetDescriptorSetAllocation().ShaderBuffers[0].GetMappedData<CameraUBO>();
			_objectBuffer = _material->GetDescriptorSetAllocation().ShaderBuffers[1].GetMappedData<ObjectBuffer>();

			const std::vector<Rendering::VertexPosition2DColorUV> quad = {{{-0.005f, 0.005f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // Top Left
																		  {{0.005f, 0.005f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // Top Right
																		  {{-0.005f, -0.005f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom Left
																		  {{0.005f, -0.005f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}}; // Bottom Right

			const std::vector<uint16_t> quadIndices = {0, 1, 2, 2, 1, 3};

			std::vector<Rendering::VertexPosition2DColorUV> vertices;
			vertices.reserve(1024 * 4);
			std::vector<uint16_t> indices;
			indices.reserve(1024 * 6);
			for (size_t i = 0; i < 1024; i++)
			{
				for (const Rendering::VertexPosition2DColorUV& vertex : quad) { vertices.push_back(vertex); }

				for (uint16_t index : quadIndices) { indices.push_back(index + (i * 6)); }
			}

			_model.reset(new Rendering::Model({reinterpret_cast<std::vector<std::byte>&>(vertices), indices}));
		}

		void Execute(TransformComponent* transformComponents, SpriteComponent* spriteComponent, std::vector<uint64_t>& entities, ECS::Context& context) final
		{
			size_t numEntities = entities.size();

			_indexes = std::ranges::iota_view((size_t) 0, entities.size());
			std::for_each(std::execution::par, _indexes.begin(), _indexes.end(), [transformComponents, spriteComponent, this](size_t i) {
				_objectBuffer->objects[i].position = {transformComponents[i].Position, spriteComponent[i].textureID};
			});

			vk::CommandBuffer commandBuffer = context.RenderingContext->GetCommandBuffer();

			_cameraUBO->view = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			_cameraUBO->proj = glm::perspective(glm::radians(45.0f),
												static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().width) /
														-static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().height),
												0.1f,
												10.0f);

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

			commandBuffer.drawIndexed(_model->GetModelBuffer().GetBufferElementNum(1), std::max(1u, static_cast<uint32_t>(numEntities) / 1024u), 0, 0, 0);
		}


	private:
		struct CameraUBO
		{
				glm::mat4 model;
				glm::mat4 view;
				glm::mat4 proj;
		};

		struct ObjectData
		{
				glm::vec4 position;
		};

		struct ObjectBuffer
		{
				std::array<ObjectData, 10'240'000> objects;
		};

		ObjectBuffer* _objectBuffer;
		CameraUBO*    _cameraUBO;

		AssetHandle<Rendering::Material> _material;

		std::unique_ptr<Rendering::Model> _model;

		std::ranges::iota_view<size_t, size_t> _indexes;
};

class PhysicsSystem : public ECS::System<TransformComponent, PhysicsComponent>
{
	public:
		PhysicsSystem() :
			ECS::System<TransformComponent, PhysicsComponent>()
		{}

		void Execute(TransformComponent* transformComponents, PhysicsComponent* physicsComponents, std::vector<uint64_t>& entities, ECS::Context& context) final
		{
			ImGui::Text("Num Entities: %llu", entities.size());

			_indexes = std::ranges::iota_view((size_t) 0, entities.size());
			std::for_each(std::execution::par, _indexes.begin(), _indexes.end(), [transformComponents, physicsComponents](size_t i) {
				transformComponents[i].Position += physicsComponents[i].Velocity;
			});
		}

		std::ranges::iota_view<size_t, size_t> _indexes;
};

int main()
{
	Application application = Application({});
	application.Initialize();

	AssetDatabase& assetDatabase = application.GetAssetDatabase();

	AssetHandle<Rendering::Texture2D> floppaTexture     = assetDatabase.CreateAsset<Rendering::Texture2D>(Texture::Floppa, {"res/textures/floppa.png", {}});
	AssetHandle<Rendering::Texture2D> evilFloppaTexture = assetDatabase.CreateAsset<Rendering::Texture2D>(Texture::EvilFloppa,
																										  {"res/textures/evil_floppa.png", {}});

	AssetHandle<Rendering::Shader> shader = assetDatabase.CreateAsset<Rendering::Shader>(Shader::Sprite, {"res/shaders/debug"});

	std::vector<AssetHandle<Rendering::Texture2D>> textures = {floppaTexture, evilFloppaTexture};

	AssetHandle<Rendering::Material> material = assetDatabase.CreateAsset<Rendering::Material>(Material::Sprite, {shader});

	material->SetTextures(0, 0, textures);

	ECS::Registry& ecs = application.GetECSRegistry();

	ecs.RegisterComponent<TransformComponent>();
	ecs.RegisterComponent<SpriteComponent>();
	ecs.RegisterComponent<PhysicsComponent>();

	ecs.RegisterGameplaySystem<PhysicsSystem>();
	ecs.RegisterRenderSystem<SpriteRenderSystem>(material);

	BENCHMARK_BEGIN

	for (int i = 0; i < 10'240'000; ++i)
	{
		ecs.CreateEntity<TransformComponent, PhysicsComponent, SpriteComponent>({glm::ballRand(0.7f)},
																				{true, {0.0f, -0.00001f, -0.001f}},
																				{(uint32_t)glm::linearRand(0, 1)});
	}

	BENCHMARK_END("Entity Creation")


	application.Run();

	return 0;
}