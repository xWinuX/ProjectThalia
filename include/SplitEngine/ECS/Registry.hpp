#pragma once

#include "SplitEngine/DataStructures.hpp"

#include "Archetype.hpp"
#include "Component.hpp"
#include "Context.hpp"
#include "Entity.hpp"
#include "Stage.hpp"
#include "SystemBase.hpp"

#include <iterator>
#include <vector>

#ifndef SE_HEADLESS
namespace SplitEngine::Rendering::Vulkan
{
	class Context;
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

			void AddQueuedEntities();
			void DestroyQueuedEntities();
			void MoveQueuedEntities();

			template<typename... T>
			uint64_t CreateEntity(T&&... args)
			{
				Archetype* archetype = GetArchetype<T...>();

				uint64_t entityID = 0;
				if (!_entityGraveyard.IsEmpty())
				{
					entityID                                     = _entityGraveyard.Pop();
					uint64_t componentIndex                      = archetype->AddEntity(entityID, std::forward<T>(args)...);
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
				Entity& entity = _sparseEntityLookup[entityID];

				return _archetypeLookup[entity.GetArchetypeIndex()]->GetComponent<T>(entity.GetComponentIndex());
			}

			template<typename... T>
			void AddComponent(uint64_t entityID, T&&... components)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				_archetypeLookup[entity.GetArchetypeIndex()]->AddComponentsToEntity<T...>(entityID, std::forward<T>(components)...);
			}

			template<typename... T>
			void RemoveComponent(uint64_t entityID)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				_archetypeLookup[entity.GetArchetypeIndex()]->RemoveComponentsFromEntity<T...>(entityID);
			}

			void DestroyEntity(uint64_t entityID);

			template<typename T>
			void RegisterComponent()
			{
				TypeIDGenerator<Component>::GetID<T>();
				_componentSizes.push_back(sizeof(T));

				_archetypeRoot->Resize();
			}

			template<typename T, typename... TArgs>
			void RegisterSystem(Stage stage, uint64_t order, TArgs&&... args)
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				_systems[static_cast<uint8_t>(stage)].push_back(new T(std::forward<TArgs>(args)...));
			}

			template<typename... T>
			Archetype* GetArchetype()
			{
				static uint64_t archetypeID = _archetypeRoot->FindArchetype<T...>()->ID;

				return _archetypeLookup[archetypeID];
			}

			bool IsEntityValid(uint64_t entityID);

			void RegisterAssetDatabase(SplitEngine::AssetDatabase* assetDatabase);


			[[nodiscard]] std::vector<Archetype*> GetArchetypesWithSignature(const DynamicBitSet& signature);

		private:
			std::vector<Entity> _sparseEntityLookup;
			std::vector<size_t> _componentSizes;

			Archetype* _archetypeRoot = nullptr;

			std::vector<Archetype*> _archetypeLookup;

			AvailableStack<uint64_t> _entityGraveyard;

			std::vector<SystemBase*> _gameplaySystems;

			std::vector<std::vector<SystemBase*>> _systems;

			Context _context {};

#ifndef SE_HEADLESS
		public:
			void RegisterRenderingContext(SplitEngine::Rendering::Vulkan::Context* context) { _context.RenderingContext = context; }

			void RegisterAudioManager(SplitEngine::Audio::Manager* audioManager) { _context.AudioManager = audioManager; }
#endif

	};


}
