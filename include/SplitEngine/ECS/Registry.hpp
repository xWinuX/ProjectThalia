#pragma once

#include "SplitEngine/DataStructures.hpp"

#include "Archetype.hpp"
#include "Component.hpp"
#include "Context.hpp"
#include "Entity.hpp"
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

			void Update(float deltaTime);

			void AddQueuedEntities();
			void DestroyQueuedEntities();
			void MoveQueuedEntities();

			void Render(float deltaTime);

			template<typename... T>
			uint64_t CreateEntity(T&&... args)
			{
				Archetype* archetype = GetArchetype<T...>();

				uint64_t entityID = 0;
				if (!_entityGraveyard.IsEmpty())
				{
					entityID                                     = _entityGraveyard.Pop();
					uint64_t componentIndex                      = archetype->AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup[entityID].archetypeIndex = archetype->ID;
					_sparseEntityLookup[entityID].componentIndex = componentIndex;
				}
				else
				{
					entityID                = _sparseEntityLookup.size();
					uint64_t componentIndex = archetype->AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup.emplace_back(archetype->ID, componentIndex);
				}

				return entityID;
			}

			template<typename... T>
			void AddComponent(uint64_t entityID, T&&... components)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				Archetype::_archetypes[entity.archetypeIndex]->AddComponentsToEntity<T...>(entityID, std::forward<T>(components)...);
			}

			template<typename... T>
			void RemoveComponent(uint64_t entityID)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				Archetype::_archetypes[entity.archetypeIndex]->RemoveComponentsFromEntity<T...>(entityID);
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
			void RegisterGameplaySystem(TArgs&&... args)
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				_gameplaySystems.emplace_back(new T(std::forward<TArgs>(args)...));
			}

			template<typename... T>
			Archetype* GetArchetype()
			{
				static uint64_t archetypeID = _archetypeRoot->FindArchetype<T...>()->ID;

				return Archetype::_archetypes[archetypeID];
			}

			void RegisterAssetDatabase(SplitEngine::AssetDatabase* assetDatabase);


			[[nodiscard]] std::vector<Archetype*> GetArchetypesWithSignature(const DynamicBitSet& signature);

		private:
			std::vector<Entity> _sparseEntityLookup;
			std::vector<size_t> _componentSizes;

			Archetype* _archetypeRoot = nullptr;

			AvailableStack<uint64_t> _entityGraveyard;

			std::vector<SystemBase*> _gameplaySystems;

			Context _context {};

#ifndef SE_HEADLESS
		public:
			void RegisterRenderingContext(SplitEngine::Rendering::Vulkan::Context* context) { _context.RenderingContext = context; }

			void RegisterAudioManager(SplitEngine::Audio::Manager* audioManager) { _context.AudioManager = audioManager; }

			template<typename T, typename... TArgs>
			void RegisterRenderSystem(TArgs&&... args)
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				_renderSystems.emplace_back(new T(std::forward<TArgs>(args)...));
			}

		private:
			std::vector<SystemBase*> _renderSystems;
#endif

	};


}
