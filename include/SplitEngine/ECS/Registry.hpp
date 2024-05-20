#pragma once

#include <set>

#include "SplitEngine/DataStructures.hpp"

#include "Archetype.hpp"
#include "Component.hpp"
#include "ContextProvider.hpp"
#include "Entity.hpp"
#include "SystemBase.hpp"

#include <vector>

namespace SplitEngine::ECS
{
	class Registry
	{
		public:
			template<typename T>
			struct SystemHandle
			{
				uint64_t ID;
				T*       System;
			};

			struct StageInfo
			{
				uint8_t Stage;
				int64_t Order;
			};

		public:
			Registry();

			~Registry();

			void PrepareForExecution();
			void ExecuteSystems();

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

			template<typename T>
			void RegisterContext(T&& context) { _contextProvider.RegisterContext<T>(std::forward<T>(context)); }

			template<typename T, typename... TArgs>
			SystemHandle<T> AddSystem(uint8_t stage, int64_t order, TArgs&&... args) { return AddSystem<T, TArgs...>({ { stage, order } }, std::forward<TArgs>(args)...); }

			template<typename T, typename... TArgs>
			SystemHandle<T> AddSystem(std::vector<StageInfo>&& stageInfos, TArgs&&... args)
			{
				static_assert(std::is_base_of_v<SystemBase, T>, "an ECS System needs to derive from SplitEngine::ECS::System");

				std::vector<SystemLocation> locations = std::vector<SystemLocation>(stageInfos.size());

				for (int i = 0; i < stageInfos.size(); ++i) { locations[i] = { stageInfos[i].Stage, stageInfos[i].Order, -1u }; }

				uint64_t systemID = _systemGraveyard.IsEmpty() ? _systems.size() : _systemGraveyard.Pop();
				if constexpr (std::is_constructible_v<T, TArgs...>) { _systems.push_back({ systemID, new T(std::forward<TArgs>(args)...), std::move(locations), false }); }
				else { _systems.push_back({ systemID, new T(std::forward<TArgs>(args)..., _contextProvider), std::move(locations), false }); }

				return { systemID, reinterpret_cast<T*>(_systems[systemID].System) };
			}

			std::vector<float>& GetAccumulatedStageTimeMs();

			std::vector<uint8_t>& GetActiveStages();

			void RemoveSystem(uint64_t systemID);

			template<typename... T>
			[[nodiscard]] Archetype* GetArchetype() const
			{
				static uint64_t archetypeID = _archetypeRoot->FindArchetype<T...>()->ID;

				return _archetypeLookup[archetypeID];
			}

			bool IsEntityValid(uint64_t entityID);

			[[nodiscard]] bool IsSystemValid(uint64_t systemID) const;

			void SetEnableStatistics(bool enabled);

			[[nodiscard]] std::vector<Archetype*> GetArchetypesWithSignature(const DynamicBitSet& signature);

			[[nodiscard]] ContextProvider& GetContextProvider();

		private:
			struct SystemLocation
			{
				uint8_t  Stage      = -1;
				int64_t  Order      = 0;
				uint64_t StageIndex = -1;
			};

			struct SystemEntry
			{
				public:
					uint64_t                    ID     = -1;
					SystemBase*                 System = nullptr;
					std::vector<SystemLocation> Locations{};
					bool                        Added = false;
			};

			struct SystemExecutionEntry
			{
				uint64_t SystemID            = -1;
				uint64_t SystemLocationIndex = -1;
			};

			std::vector<Entity>    _sparseEntityLookup{};
			std::vector<Component> _sparseComponentLookup{};

			Archetype* _archetypeRoot = nullptr;

			std::vector<Archetype*> _archetypeLookup{};

			AvailableStack<uint64_t> _entityGraveyard{};

			uint64_t _systemID = 0;

			AvailableStack<uint64_t> _systemGraveyard{};

			std::vector<SystemEntry>                       _systems             = std::vector<SystemEntry>();
			std::vector<std::vector<SystemExecutionEntry>> _systemExecutionFlow = std::vector<std::vector<SystemExecutionEntry>>(std::numeric_limits<uint8_t>::max());
			std::vector<uint8_t>                           _activeStages        = std::vector<uint8_t>();

			std::set<uint8_t>    _tmpActiveStages       = std::set<uint8_t>();
			std::vector<uint8_t> _tmpStagesToDeactivate = std::vector<uint8_t>();

			std::vector<uint64_t> _systemsToRemove{};

			ContextProvider _contextProvider{};

			bool               _collectStatistics      = false;
			std::vector<float> _accumulatedStageTimeMs = std::vector<float>(std::numeric_limits<uint8_t>::max(), 0);

			void AddQueuedSystems();

			void RemoveQueuedSystems();

			SystemLocation& GetSystemLocationOfExecutionEntry(const SystemExecutionEntry& executionEntry);
	};
}
