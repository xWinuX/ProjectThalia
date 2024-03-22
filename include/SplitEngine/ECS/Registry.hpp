#pragma once

#include "SplitEngine/DataStructures.hpp"

#include "Archetype.hpp"
#include "Component.hpp"
#include "Context.hpp"
#include "Entity.hpp"
#include "Stage.hpp"
#include "SystemBase.hpp"

#include <vector>

#ifndef SE_HEADLESS
namespace SplitEngine::Rendering::Vulkan
{
	class Instance;
}
#endif

namespace SplitEngine
{
	class AssetDatabase;
}

namespace SplitEngine::ECS
{
	class Registry
	{
		public:
			Registry();

			~Registry();

			void PrepareForExecution(float deltaTime);

			void ExecuteSystems(Stage stageToExecute);

			template<typename... T>
			uint64_t CreateEntity(T&&... args)
			{
				Archetype* archetype = GetArchetype<T...>();

				uint64_t entityID = 0;
				if (!_entityGraveyard.IsEmpty())
				{
					entityID                                         = _entityGraveyard.Pop();
					const uint64_t componentIndex                    = archetype->AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup[entityID].moveArchetypeIndex = archetype->ID;
					_sparseEntityLookup[entityID].moveComponentIndex = componentIndex;
				}
				else
				{
					entityID                = _sparseEntityLookup.size();
					uint64_t componentIndex = archetype->AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup.emplace_back(-1, -1, archetype->ID, componentIndex);
				}

				return entityID;
			}

			template<typename T>
			T& GetComponent(uint64_t entityID)
			{
				const Entity& entity = _sparseEntityLookup[entityID];

				return _archetypeLookup[entity.GetArchetypeIndex()]->GetComponent<T>(entity.GetComponentIndex());
			}

			template<typename... T>
			void AddComponent(uint64_t entityID, T&&... components)
			{
				const Entity& entity = _sparseEntityLookup[entityID];

				_archetypeLookup[entity.GetArchetypeIndex()]->AddComponentsToEntity<T...>(entityID, std::forward<T>(components)...);
			}

			template<typename... T>
			void RemoveComponent(const uint64_t entityID) const
			{
				const Entity& entity = _sparseEntityLookup[entityID];

				_archetypeLookup[entity.GetArchetypeIndex()]->RemoveComponentsFromEntity<T...>(entityID);
			}

			void DestroyEntity(uint64_t entityID);

			template<typename T>
			void RegisterComponent()
			{
				TypeIDGenerator<Component>::GetID<T>();

				_sparseComponentLookup.push_back({
					                                 sizeof(T),
					                                 [](std::byte* rawComponent)
					                                 {
						                                 T* component = reinterpret_cast<T*>(rawComponent);
						                                 component->~T();
					                                 }
				                                 });

				_archetypeRoot->Resize();
			}

			template<typename T, typename... TArgs>
			uint64_t AddSystem(Stage stage, int64_t order, TArgs&&... args)
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				uint64_t systemID;
				if (_systemGraveyard.IsEmpty())
				{
					systemID = _systemID++;
					_sparseSystemLookup.push_back({ stage, _systemsToAdd[static_cast<uint64_t>(stage)].size() });
				}
				else
				{
					systemID                      = _systemGraveyard.Pop();
					_sparseSystemLookup[systemID] = { stage, _systemsToAdd[static_cast<uint64_t>(stage)].size() };
				}

				_systemsToAdd[static_cast<uint64_t>(stage)].push_back({ systemID, new T(std::forward<TArgs>(args)...), order });

				return systemID;
			}

			void RemoveSystem(uint64_t systemID);

			template<typename... T>
			[[nodiscard]] Archetype* GetArchetype() const
			{
				static uint64_t archetypeID = _archetypeRoot->FindArchetype<T...>()->ID;

				return _archetypeLookup[archetypeID];
			}

			bool IsEntityValid(uint64_t entityID);

			bool IsSystemValid(uint64_t systemID);

			void RegisterApplication(Application* application);

			void RegisterAssetDatabase(AssetDatabase* assetDatabase);

			[[nodiscard]] std::vector<Archetype*> GetArchetypesWithSignature(const DynamicBitSet& signature);

		private:
			std::vector<Entity>    _sparseEntityLookup{};
			std::vector<Component> _sparseComponentLookup{};

			Archetype* _archetypeRoot = nullptr;

			std::vector<Archetype*> _archetypeLookup{};

			AvailableStack<uint64_t> _entityGraveyard{};

			struct SystemEntry
			{
				public:
					uint64_t    ID     = -1;
					SystemBase* System = nullptr;
					int64_t     Order  = 0;
			};

			struct SystemLookupEntry
			{
				public:
					Stage    Stage = Stage::MAX_VALUE;
					uint64_t Index = -1;
					bool     Added = false;
			};

			uint64_t _systemID = 0;

			std::vector<SystemLookupEntry> _sparseSystemLookup{};
			AvailableStack<uint64_t>       _systemGraveyard{};

			std::vector<std::vector<SystemEntry>> _systems = std::vector<std::vector<SystemEntry>>(static_cast<uint8_t>(Stage::MAX_VALUE), std::vector<SystemEntry>());

			std::vector<std::vector<SystemEntry>> _systemsToAdd = std::vector<std::vector<SystemEntry>>(static_cast<uint8_t>(Stage::MAX_VALUE), std::vector<SystemEntry>());

			std::vector<uint64_t> _systemsToRemove{};

			Context _context{};

			void AddQueuedSystems();
			void RemoveQueuedSystems();

#ifndef SE_HEADLESS

		public:
			void RegisterRenderingContext(Rendering::Vulkan::Instance* context) { _context.RenderingContext = context; }

			void RegisterAudioManager(Audio::Manager* audioManager) { _context.AudioManager = audioManager; }
#endif
	};
}
