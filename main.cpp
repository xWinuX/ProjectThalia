#include "SplitEngine/Application.hpp"
#include "SplitEngine/AssetDatabase.hpp"
#include "SplitEngine/Debug/Performance.hpp"
#include "SplitEngine/ECS/System.hpp"
#include "SplitEngine/IO/AudioLoader.hpp"
#include "SplitEngine/Input.hpp"
#include "SplitEngine/Rendering/Sprite.hpp"
#include "SplitEngine/Rendering/Texture2D.hpp"
#include "SplitEngine/Tools/ImagePacker.hpp"
#include "SplitEngine/Tools/ImageSlicer.hpp"
#include "imgui.h"

#include <execution>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

using namespace SplitEngine;


enum class InputAction
{
	Move,
	Rotate,
	Fire,
};

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

enum class SoundEffect
{
	Hey,
};

/**
 * Components should only contain data
 * The easier to copy the better the performance!
 */
struct TransformComponent
{
		glm::vec3 Position {};
		float     Rotation = 0;
};

struct SpriteComponent
{
		AssetHandle<Rendering::Sprite> Sprite;
		float                          AnimationSpeed    = 0;
		float                          CurrentFrame      = 0;
		uint32_t                       PreviousTextureID = 0;
};

struct AudioSourceComponent
{
		AssetHandle<Audio::SoundEffect> SoundEffect;
		bool                            Play = false;
};

struct PhysicsComponent
{
		bool      HasGravity = false;
		glm::vec3 Velocity {};
};

struct PlayerComponent
{};

struct CameraComponent
{
		uint64_t TargetEntity;
};

class CameraSystem : public ECS::System<TransformComponent, CameraComponent>
{
	public:
		CameraSystem() :
			ECS::System<TransformComponent, CameraComponent>()
		{}

		void Execute(TransformComponent*        transformComponents,
					 CameraComponent*           cameraComponents,
					 std::vector<uint64_t>&     entities,
					 SplitEngine::ECS::Context& context) override
		{
			CameraUBO* cameraUBO = Rendering::Shader::GetGlobalProperties().GetBuffer<CameraUBO>(0);
			for (int i = 0; i < entities.size(); ++i)
			{
				TransformComponent& transformComponent = transformComponents[i];
				CameraComponent&    cameraComponent    = cameraComponents[i];

				if (context.Registry->IsEntityValid(cameraComponent.TargetEntity))
				{
					transformComponent.Position = context.Registry->GetComponent<TransformComponent>(cameraComponent.TargetEntity).Position -
												  glm::vec3(0.0f, 0.0f, 10.0f);

					uint32_t width  = Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().width;
					uint32_t height = Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().height;

					cameraUBO->view = glm::lookAt(transformComponent.Position,
												  transformComponent.Position + glm::vec3(0.0f, 0.0f, 1.0f),
												  glm::vec3(0.0f, 1.0f, 0.0f));

					cameraUBO->proj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / -static_cast<float>(height), 0.1f, 1000.0f);

					cameraUBO->viewProj = cameraUBO->proj * cameraUBO->view;

					glm::ivec2 inputMouseOffset = transformComponent.Position;
					inputMouseOffset -= glm::ivec2(static_cast<int>(width / 2u), static_cast<int>(height / 2u));

					Input::ProvideWorldMouseOffset(inputMouseOffset);
				}
			}
		}

	private:
		struct CameraUBO
		{
				glm::mat4 view;
				glm::mat4 proj;
				glm::mat4 viewProj;
		};
};

class PlayerSystem : public ECS::System<TransformComponent, PlayerComponent, PhysicsComponent>
{
	public:
		PlayerSystem(AssetHandle<Rendering::Sprite> bulletSprite) :
			_bulletSprite(bulletSprite),
			ECS::System<TransformComponent, PlayerComponent, PhysicsComponent>()
		{}

		void Execute(TransformComponent*        transformComponents,
					 PlayerComponent*           playerComponents,
					 PhysicsComponent*          physicsComponents,
					 std::vector<uint64_t>&     entities,
					 SplitEngine::ECS::Context& context) override
		{
			for (int i = 0; i < entities.size(); ++i)
			{
				TransformComponent& transformComponent = transformComponents[i];
				PlayerComponent&    playerComponent    = playerComponents[i];
				PhysicsComponent&   physicsComponent   = physicsComponents[i];

				glm::vec2 inputAxis = Input::GetAxis2DActionDown(InputAction::Move);

				glm::vec2 direction = glm::normalize(glm::vec3(Input::GetMousePosition(), 0.0f) - transformComponent.Position);

				transformComponent.Rotation = glm::degrees(glm::atan(direction.y, direction.x));
				physicsComponent.Velocity   = glm::vec3(inputAxis * 50.0f, 0.0f);

				if (Input::GetButtonActionDown(InputAction::Fire))
				{

					context.Registry->CreateEntity<TransformComponent, PhysicsComponent, SpriteComponent>({transformComponent.Position,
																										   transformComponent.Rotation},
																										  {false, glm::vec3(direction.x, -direction.y, 0.0f)},
																										  {_bulletSprite});
				}
			}
		}

	private:
		AssetHandle<Rendering::Sprite> _bulletSprite;
};

class AudioSystem : public ECS::System<AudioSourceComponent>
{
	public:
		AudioSystem() :
			ECS::System<AudioSourceComponent>()
		{}

		void Execute(AudioSourceComponent* audioSourceComponents, std::vector<uint64_t>& entities, SplitEngine::ECS::Context& context) override
		{
			_indexes = std::ranges::iota_view((size_t) 0, entities.size());
			std::for_each(std::execution::par, _indexes.begin(), _indexes.end(), [audioSourceComponents, context](size_t i) {
				AssetHandle<Audio::SoundEffect>& soundEffect = audioSourceComponents[i].SoundEffect;
				if (audioSourceComponents[i].Play)
				{
					context.AudioManager->PlaySound(soundEffect, 1.0f);
					audioSourceComponents[i].Play = false;
				}
			});
		}

	private:
		std::ranges::iota_view<size_t, size_t> _indexes;
};

// Systems can be as complex as you want and can contain a lot of state
class SpriteRenderSystem : public ECS::System<TransformComponent, SpriteComponent>
{
	public:
		SpriteRenderSystem(AssetHandle<Rendering::Material> material, Tools::ImagePacker::PackingData& packingData) :
			_material(material),
			ECS::System<TransformComponent, SpriteComponent>()
		{
			// Get buffers
			TextureStore* textureStore = _material->GetShader()->GetProperties().GetBuffer<TextureStore>(0);

			float limit = std::numeric_limits<float>::max();

			auto rawBuffer = _material->GetProperties().GetBufferRaw<ObjectBuffer>(0);

			glm::vec4 farAwayPos = glm::vec4(limit, limit, limit, 0.0f);

			for (int i = 0; i < Rendering::Vulkan::Device::MAX_FRAMES_IN_FLIGHT; ++i)
			{
				ObjectBuffer* b = reinterpret_cast<ObjectBuffer*>(rawBuffer[i]);
				for (auto& position : b->positions) { position = farAwayPos; }
			}

			// Prepare texture page info data for shader upload
			for (int i = 0; i < packingData.PackingInfos.size(); ++i)
			{
				Tools::ImagePacker::PackingInfo packingInfo       = packingData.PackingInfos[i];
				textureStore->Textures[i].PageIndexAndAspectRatio = {packingInfo.PageIndex, packingInfo.AspectRatio};
				textureStore->Textures[i].UVs[0]                  = {packingInfo.UVTopLeft, 0.0f, 0.0f};
				textureStore->Textures[i].UVs[1]                  = {packingInfo.UVTopRight, 0.0f, 0.0f};
				textureStore->Textures[i].UVs[2]                  = {packingInfo.UVBottomLeft, 0.0f, 0.0f};
				textureStore->Textures[i].UVs[3]                  = {packingInfo.UVBottomRight, 0.0f, 0.0f};
			}

			// Actually create shader pages
			for (const auto& pageImage : packingData.PageImages)
			{
				_texturePages.push_back(std::make_unique<Rendering::Texture2D, Rendering::Texture2D::CreateInfo>({pageImage, {}}));
			}

			// Send texture pages to shader
			_material->GetShader()->GetProperties().SetTextures(1, 0, _texturePages);

			const std::vector<uint32_t> quad        = {0, 1, 2, 2, 1, 3};
			const std::vector<uint16_t> quadIndices = {0, 1, 2, 3, 4, 5};

			std::vector<uint32_t> vertices;
			vertices.reserve(NUM_QUADS_IN_BATCH * 6);
			std::vector<uint16_t> indices;
			indices.reserve(NUM_QUADS_IN_BATCH * 6);
			for (size_t i = 0; i < NUM_QUADS_IN_BATCH; i++)
			{
				for (const uint32_t& vertex : quad) { vertices.push_back(vertex); }

				for (uint16_t index : quadIndices) { indices.push_back(index + (i * 6)); }
			}

			_model = std::make_unique<Rendering::Model, Rendering::Model::CreateInfo>({reinterpret_cast<std::vector<std::byte>&>(vertices), indices});
		}

		/**
		 * This function can be override if you need more control over how archetypes are handled
		 *
		 * In this example we want to apply animation and positions to all entities with sprite and transform components.
		 * Since this is a render system we also want to give the gpu the command to draw these sprites, but if we do it in the execute function
		 * each Archetype will causes a draw call which is not optimal, so we just apply animation and transforms in the execute function
		 * and issue the draw call here so we can render all sprites in 1 draw call
		 *
		 * If you don't override this function it will call the execute function with all required parameters automatically thanks to template magic!
		 */
		void ExecuteArchetypes(std::vector<ECS::Archetype*>& archetypes, ECS::Context& context) override
		{
			size_t numEntities = 0;

			ObjectBuffer* objectBuffer = _material->GetProperties().GetBuffer<ObjectBuffer>(0);

			for (const auto& archetype : archetypes)
			{
				TransformComponent* transformComponents = archetype->GetComponents<TransformComponent>();
				SpriteComponent*    spriteComponents    = archetype->GetComponents<SpriteComponent>();

				std::vector<uint64_t>& entities = archetype->Entities;

				memcpy(objectBuffer->positions.data() + numEntities, transformComponents, entities.size() * sizeof(glm::vec4));

				_indexes = std::ranges::iota_view((size_t) 0, entities.size());
				std::for_each(std::execution::par, _indexes.begin(), _indexes.end(), [this, objectBuffer, spriteComponents, context, &numEntities](size_t i) {
					SpriteComponent&   spriteAnimatorComponent = spriteComponents[i];
					Rendering::Sprite* sprite                  = spriteAnimatorComponent.Sprite.Get();
					size_t             numSubSprites           = sprite->_numSubSprites;
					float              animationSpeed          = spriteAnimatorComponent.AnimationSpeed;

					if (numSubSprites > 1 && animationSpeed > 0.0f)
					{
						float currentFrame     = spriteAnimatorComponent.CurrentFrame;
						float animationAdvance = animationSpeed * context.DeltaTime;

						float    newCurrentFrame     = currentFrame + animationAdvance;
						uint32_t castNewCurrentFrame = static_cast<uint32_t>(newCurrentFrame);

						if (castNewCurrentFrame >= numSubSprites)
						{
							spriteAnimatorComponent.CurrentFrame = FastFmod(newCurrentFrame, static_cast<float>(numSubSprites));
						}
						else { spriteAnimatorComponent.CurrentFrame = newCurrentFrame; }

						objectBuffer->textureIDs[numEntities + i] = sprite->GetTextureID(static_cast<uint32_t>(spriteAnimatorComponent.CurrentFrame));
					}
					else { objectBuffer->textureIDs[numEntities + i] = sprite->GetTextureID(0); }
				});

				numEntities += entities.size();
			}

			objectBuffer->numObjects = numEntities;

			vk::CommandBuffer commandBuffer = context.RenderingContext->GetCommandBuffer();

			// Bind global
			Rendering::Shader::UpdateGlobal();

			_material->GetShader()->BindGlobal(commandBuffer);

			// Bind shader specific
			_material->GetShader()->Update();

			_material->GetShader()->Bind(commandBuffer);

			// Bind material specific
			_material->Update();

			_material->Bind(commandBuffer);

			_model->Bind(commandBuffer);

			uint32_t numInstances = std::max(1u, static_cast<uint32_t>(std::ceil(static_cast<float>(numEntities) / static_cast<float>(NUM_QUADS_IN_BATCH))));
			commandBuffer.drawIndexed(_model->GetModelBuffer().GetBufferElementNum(1), numInstances, 0, 0, 0);
		}


	private:
		static constexpr uint32_t NUM_QUADS_IN_BATCH = 10240;

		AssetHandle<Rendering::Material> _material;

		std::unique_ptr<Rendering::Model> _model;

		static inline float FastFmod(float a, float b) { return ((a) - ((int) ((a) / (b))) * (b)); }

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
				std::array<glm::vec4, 5'120'000> positions;
				std::array<uint32_t, 5'120'000>  textureIDs;
				uint32_t                         numObjects;
		};

		std::vector<std::unique_ptr<Rendering::Texture2D>> _texturePages;

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
		 * The execute function contains a pointer to each component array used by the system
		 * This function can run multiple times and is called for each archetype contains all components needed by this system
		 */
		void Execute(TransformComponent* transformComponents, PhysicsComponent* physicsComponents, std::vector<uint64_t>& entities, ECS::Context& context) final
		{
			_indexes = std::ranges::iota_view((size_t) 0, entities.size());
			std::for_each(std::execution::par, _indexes.begin(), _indexes.end(), [transformComponents, physicsComponents, entities, context](size_t i) {
				transformComponents[i].Position += physicsComponents[i].Velocity * context.DeltaTime;
			});

			ImGui::Text("Num Entities: %llu", entities.size());
		}

		std::ranges::iota_view<size_t, size_t> _indexes;
};

int main()
{
	Application application = Application({});
	application.Initialize();

	Input::RegisterAxis2D(InputAction::Move, {KeyCode::A, KeyCode::D}, {KeyCode::S, KeyCode::W});
	Input::RegisterButtonAction(InputAction::Fire, KeyCode::MOUSE_LEFT);

	AssetDatabase& assetDatabase = application.GetAssetDatabase();

	/**
	 * You can create an Asset with every type that has a public struct named CreateInfo!
	 * That means you can also create custom assets
	 * The key can be anything as long as it can be cast into an int, so no magic strings!
	 */
	LOG("Before Create shader");
	AssetHandle<Rendering::Shader> shader = assetDatabase.CreateAsset<Rendering::Shader>(Shader::Sprite, {"res/shaders/debug"});
	LOG("After  Create shader");

	AssetHandle<Rendering::Material> material = assetDatabase.CreateAsset<Rendering::Material>(Material::Sprite, {shader});

	AssetHandle<Audio::SoundEffect> hey = assetDatabase.CreateAsset<Audio::SoundEffect>(SoundEffect::Hey,
																						{IO::AudioLoader::LoadStream("res/audio/CarstenBlasterSound.wav"),
																						 1.0f});

	// Create texture page and sprite assets
	Tools::ImagePacker texturePacker = Tools::ImagePacker();

	uint64_t floppaPackerID = texturePacker.AddRelatedImages(Tools::ImageSlicer::Slice("res/textures/floppa.png", {2}));
	uint64_t salicePackerID = texturePacker.AddRelatedImages(Tools::ImageSlicer::Slice("res/textures/salice_move_front.png", {5}));

	Tools::ImagePacker::PackingData packingData = texturePacker.Pack(2048);

	AssetHandle<Rendering::Sprite> floppaSprite = assetDatabase.CreateAsset<Rendering::Sprite>(Sprite::Floppa, {floppaPackerID, packingData});
	AssetHandle<Rendering::Sprite> saliceSprite = assetDatabase.CreateAsset<Rendering::Sprite>(Sprite::Salice, {salicePackerID, packingData});


	LOG("floppa num sub {0}", floppaSprite->_numSubSprites);

	// Setup ECS
	ECS::Registry& ecs = application.GetECSRegistry();

	/**
	 * 	Each component that is used needs to be registered
	 * 	It's important that every Component, that will be used, is registered before any Entities are created or else the ECS will not work or likely crash the app
	 */
	ecs.RegisterComponent<TransformComponent>();
	ecs.RegisterComponent<SpriteComponent>();
	ecs.RegisterComponent<PhysicsComponent>();
	ecs.RegisterComponent<AudioSourceComponent>();
	ecs.RegisterComponent<PlayerComponent>();
	ecs.RegisterComponent<CameraComponent>();

	/**
	 * 	Each system can either be registered as a gameplay system or as a render system
	 * 	Gameplay systems execute before rendering systems and don't have any special context
	 * 	Render systems execute after gameplay systems and run in a vulkan context so draw calls/binds etc... can be made
	 * 	Systems can be registered multiple times and parameters get forwarded to the systems constructor
	 * 	Systems can be registered as the game is running, so no need to preregister all at the start if you don't want to
	 */

	ecs.RegisterSystem<PhysicsSystem>(ECS::Stage::Gameplay, 0);
	ecs.RegisterSystem<AudioSystem>(ECS::Stage::Gameplay, 0);
	ecs.RegisterSystem<PlayerSystem>(ECS::Stage::Gameplay, 0, floppaSprite);
	ecs.RegisterSystem<CameraSystem>(ECS::Stage::Gameplay, 0);
	ecs.RegisterSystem<SpriteRenderSystem>(ECS::Stage::Rendering, 0, material, packingData);

	constexpr uint64_t numEntities   = 2'000'000;
	constexpr uint64_t numIterations = 0;

	LOG("---Begin ECS Benchmark---");
	LOG("Benchmarking with {0} iterations", numEntities);

	std::vector<uint64_t> _entityIDs = std::vector<uint64_t>(numEntities, -1);

	for (int iteration = 0; iteration < numIterations; ++iteration)
	{
		LOG("");
		LOG("-- {0} --", iteration);
		LOG("-Begin Entity Create Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			_entityIDs[i] = ecs.CreateEntity<TransformComponent, PhysicsComponent>({{591.0f, 31245.0f, -553.0f}, 0.0f}, {true, {591.0f, 31245.0f, -553.0f}});
		}
		BENCHMARK_END("Initial Creation Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual creation time")

		LOG("");
		LOG("-Begin Entity Component Remove Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]); }
		BENCHMARK_END("Initial Remove Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Remove Time")

		LOG("");
		LOG("-Begin Entity Component Add Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.AddComponent<PhysicsComponent>(_entityIDs[i], {true, {591.0f, 31245.0f, -553.0f}}); }
		BENCHMARK_END("Initial Add Call time")

		BENCHMARK_BEGIN
		LOG("move");
		ecs.MoveQueuedEntities();
		LOG("add");
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add Time")

		LOG("");
		LOG("-Begin Entity Component Remove Multiple Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.RemoveComponent<TransformComponent>(_entityIDs[i]);
			ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]);
		}
		BENCHMARK_END("Initial Remove Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Remove Multiple Time")

		LOG("");
		LOG("-Begin Entity Component Add Multiple Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.AddComponent<TransformComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f});
			ecs.AddComponent<PhysicsComponent>(_entityIDs[i], {true, {591.0f, 31245.0f, -553.0f}});
		}
		BENCHMARK_END("Initial Add Multiple Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add Time")

		LOG("");
		LOG("-Begin Entity Component Remove Multiple (Batched) Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.RemoveComponent<TransformComponent, PhysicsComponent>(_entityIDs[i]); }
		BENCHMARK_END("Initial Remove (Batched) Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Remove Multiple (Batched) Time")

		LOG("");
		LOG("-Begin Entity Component Add Multiple (Batched) Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.AddComponent<TransformComponent, PhysicsComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f}, {true, {591.0f, 31245.0f, -553.0f}});
		}
		BENCHMARK_END("Initial Add Multiple (Batched) Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add (Batched) Time")

		LOG("");
		LOG("-Begin Entity Component Add/Remove Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.RemoveComponent<TransformComponent>(_entityIDs[i]);
			ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]);

			ecs.AddComponent<TransformComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f});
			ecs.AddComponent<PhysicsComponent>(_entityIDs[i], {true, {591.0f, 31245.0f, -553.0f}});

			ecs.RemoveComponent<TransformComponent>(_entityIDs[i]);

			ecs.AddComponent<PlayerComponent>(_entityIDs[i], {});

			ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]);

			ecs.AddComponent<TransformComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f});
		}
		BENCHMARK_END("Initial Add/Remove Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add (Batched) Time")

		LOG("");
		LOG("-Begin Entity Destroy Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.DestroyEntity(_entityIDs[i]); }
		BENCHMARK_END("Initial Destroy Call time")

		BENCHMARK_BEGIN
		ecs.DestroyQueuedEntities();
		BENCHMARK_END("Actual Destroy Time")

		LOG("-Begin Entity Create Split Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			_entityIDs[i] = ecs.CreateEntity();
			ecs.AddComponent<TransformComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f});
			ecs.AddComponent<PhysicsComponent>(_entityIDs[i], {true, {591.0f, 31245.0f, -553.0f}});
		}
		BENCHMARK_END("Initial Creation Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual creation time")

		LOG("");
		LOG("-Begin Entity Component Remove Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]); }
		BENCHMARK_END("Initial Remove Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Remove Time")

		LOG("");
		LOG("-Begin Entity Component Add Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.AddComponent<PhysicsComponent>(_entityIDs[i], {true, {591.0f, 31245.0f, -553.0f}}); }
		BENCHMARK_END("Initial Add Call time")

		BENCHMARK_BEGIN
		LOG("move");
		ecs.MoveQueuedEntities();
		LOG("add");
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add Time")

		LOG("");
		LOG("-Begin Entity Component Remove Multiple Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.RemoveComponent<TransformComponent>(_entityIDs[i]);
			ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]);
		}
		BENCHMARK_END("Initial Remove Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Remove Multiple Time")

		LOG("");
		LOG("-Begin Entity Component Add Multiple Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.AddComponent<TransformComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f});
			ecs.AddComponent<PhysicsComponent>(_entityIDs[i], {true, {591.0f, 31245.0f, -553.0f}});
		}
		BENCHMARK_END("Initial Add Multiple Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add Time")

		LOG("");
		LOG("-Begin Entity Component Remove Multiple (Batched) Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.RemoveComponent<TransformComponent, PhysicsComponent>(_entityIDs[i]); }
		BENCHMARK_END("Initial Remove (Batched) Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Remove Multiple (Batched) Time")

		LOG("");
		LOG("-Begin Entity Component Add Multiple (Batched) Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.AddComponent<TransformComponent, PhysicsComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f}, {true, {591.0f, 31245.0f, -553.0f}});
		}
		BENCHMARK_END("Initial Add Multiple (Batched) Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add (Batched) Time")

		LOG("");
		LOG("-Begin Entity Component Add/Remove Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i)
		{
			ecs.RemoveComponent<TransformComponent>(_entityIDs[i]);
			ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]);

			ecs.AddComponent<TransformComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f});
			ecs.AddComponent<PhysicsComponent>(_entityIDs[i], {true, {591.0f, 31245.0f, -553.0f}});

			ecs.RemoveComponent<TransformComponent>(_entityIDs[i]);

			ecs.AddComponent<PlayerComponent>(_entityIDs[i], {});

			ecs.RemoveComponent<PhysicsComponent>(_entityIDs[i]);

			ecs.AddComponent<TransformComponent>(_entityIDs[i], {{591.0f, 31245.0f, -553.0f}, 0.0f});
		}
		BENCHMARK_END("Initial Add/Remove Call time")

		BENCHMARK_BEGIN
		ecs.MoveQueuedEntities();
		ecs.AddQueuedEntities();
		BENCHMARK_END("Actual Add (Batched) Time")

		LOG("");
		LOG("-Begin Entity Destroy Benchmark-");
		BENCHMARK_BEGIN
		for (int i = 0; i < numEntities; ++i) { ecs.DestroyEntity(_entityIDs[i]); }
		BENCHMARK_END("Initial Destroy Call time")

		BENCHMARK_BEGIN
		ecs.DestroyQueuedEntities();
		BENCHMARK_END("Actual Destroy Time")
	}


	// Create entities
	for (int i = 0; i < 1'024'000; ++i) { ecs.CreateEntity<TransformComponent, SpriteComponent>({glm::ballRand(100.0f), 0.0f}, {floppaSprite, 1.0f, 0}); }


	uint64_t playerEntity = ecs.CreateEntity<TransformComponent, PhysicsComponent, PlayerComponent, SpriteComponent>({}, {}, {}, {floppaSprite, 1.0f});
	ecs.CreateEntity<TransformComponent, CameraComponent>({}, {playerEntity});


	// Run Game
	application.Run();


	return 0;
}