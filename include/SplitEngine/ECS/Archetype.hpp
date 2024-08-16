#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "SplitEngine/DataStructures.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

namespace SplitEngine::ECS
{
	class Archetype
	{
		friend class Registry;

		public:
			uint64_t                            ID = 0;
			std::vector<uint64_t>               Entities{};
			std::vector<std::vector<std::byte>> ComponentData{};
			std::vector<uint64_t>               ComponentIDs{};
			DynamicBitSet                       Signature{};

			Archetype(std::vector<Entity>&      sparseEntityLookup,
			          std::vector<Component>&   sparseComponentLookup,
			          std::vector<Archetype*>&  archetypeLookup,
			          AvailableStack<uint64_t>& entityGraveyard,
			          std::vector<uint64_t>&&   componentIDs);

			template<typename T>
			std::vector<std::byte>& GetComponentsRaw() { return ComponentData[TypeIDGenerator<Component>::GetID<T>()]; }

			template<class T>
			T* GetComponents() { return reinterpret_cast<T*>(ComponentData[TypeIDGenerator<Component>::GetID<T>()].data()); }

			template<class T>
			T* GetMoveComponents() { return reinterpret_cast<T*>(_componentDataToAdd[TypeIDGenerator<Component>::GetID<T>()].data()); }

			template<typename T>
			T& GetComponent(const Entity& entity)
			{
				return entity.componentIndex == -1ull ? GetMoveComponents<T>()[entity.moveComponentIndex] : GetComponents<T>()[entity.componentIndex];
			}

			template<typename... TArgs>
			uint64_t AddEntity(uint64_t entityID, TArgs&&... components)
			{
				_entitiesToAdd.push_back(entityID);

				AddComponents(std::forward<TArgs>(components)...);

				return (Entities.size() + _entitiesToAdd.size()) - 1;
			}

			void DestroyEntity(uint64_t entityID);

			template<typename... TArgs>
			Archetype* FindArchetype()
			{
				if (sizeof...(TArgs) == 0) { return this; }

				uint64_t index = -1;
				( [&]
				{
					if (index != -1) { index = _archetypeLookup[index]->GetAddArchetypeID<TArgs>(); }
					else { index = GetAddArchetypeID<TArgs>(); }
				}(), ...);

				return _archetypeLookup[index];
			}

			template<typename T>
			uint64_t GetAddArchetypeID()
			{
				const uint64_t componentIDToAdd = TypeIDGenerator<Component>::GetID<T>();

				if (_sparseAddComponentArchetypes[componentIDToAdd] == -1ull)
				{
					std::vector<uint64_t> componentIds = std::vector<uint64_t>(ComponentIDs);
					componentIds.push_back(componentIDToAdd);
					const Archetype* archetype = new Archetype(_sparseEntityLookup, _sparseComponentLookup, _archetypeLookup, _entityGraveyard, std::move(componentIds));
					_sparseAddComponentArchetypes[componentIDToAdd] = archetype->ID;
					return archetype->ID;
				}
				return _sparseAddComponentArchetypes[componentIDToAdd];
			}

			template<typename T>
			uint64_t GetRemoveArchetypeID()
			{
				const uint64_t componentIDToRemove = TypeIDGenerator<Component>::GetID<T>();
				if (_sparseRemoveComponentArchetypes[componentIDToRemove] == -1ull)
				{
					std::vector<uint64_t> componentIds;
					componentIds.reserve(ComponentIDs.size() - 1);

					for (uint64_t& componentID: ComponentIDs) { if (componentID != componentIDToRemove) { componentIds.push_back(componentID); } }

					Archetype* archetype = new Archetype(_sparseEntityLookup, _sparseComponentLookup, _archetypeLookup, _entityGraveyard, std::move(componentIds));
					_sparseRemoveComponentArchetypes[componentIDToRemove] = archetype->ID;
					archetype->_sparseAddComponentArchetypes[componentIDToRemove] = ID;
					return archetype->ID;
				}
				return _sparseRemoveComponentArchetypes[componentIDToRemove];
			}

			template<typename... TArgs>
			void RemoveComponentsFromEntity(const uint64_t entityID)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				const uint64_t oldArchetypeMoveIndex = entity.moveArchetypeIndex;

				// Recursively search for archetype in tree
				( [&]
				{
					if (entity.moveArchetypeIndex != -1ull) { entity.moveArchetypeIndex = _archetypeLookup[entity.moveArchetypeIndex]->GetRemoveArchetypeID<TArgs>(); }
					else { entity.moveArchetypeIndex = GetRemoveArchetypeID<TArgs>(); }
				}(), ...);

				if (oldArchetypeMoveIndex == -1ull) { _entitiesToMove.push_back(entityID); }
				else if (entity.moveComponentIndex != -1ull)
				{
					Archetype* newArchetype = _archetypeLookup[entity.moveArchetypeIndex];
					Archetype* oldArchetype = _archetypeLookup[oldArchetypeMoveIndex];

					newArchetype->_entitiesToAdd.push_back(entityID);

					newArchetype->ResizeAddComponentsForNewEntity();

					// Add data from old archetype to new one
					for (const auto& componentID: newArchetype->ComponentIDs)
					{
						std::vector<std::byte>& bytes         = newArchetype->_componentDataToAdd[componentID];
						const size_t&           componentSize = _sparseComponentLookup[componentID].Size;
						std::byte*              it            = oldArchetype->_componentDataToAdd[componentID].data() + (entity.moveComponentIndex * componentSize);

						std::move(std::make_move_iterator(it), std::make_move_iterator(it + componentSize), bytes.end() - componentSize);
					}

					// Remove old data form add arrays
					oldArchetype->DestroyEntityInAddQueueImmediately(entityID, true);

					entity.moveComponentIndex = newArchetype->_entitiesToAdd.size() - 1;
				}
			}

			template<typename... TArgs>
			void AddComponentsToEntity(uint64_t entityID, TArgs&&... newComponentData)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				const uint64_t oldArchetypeMoveIndex = entity.moveArchetypeIndex;

				// Recursively search for archetype in tree
				( [&]
				{
					if (entity.moveArchetypeIndex != -1) { entity.moveArchetypeIndex = _archetypeLookup[entity.moveArchetypeIndex]->GetAddArchetypeID<TArgs>(); }
					else { entity.moveArchetypeIndex = GetAddArchetypeID<TArgs>(); }
				}(), ...);


				Archetype* newArchetype = _archetypeLookup[entity.moveArchetypeIndex];

				if (oldArchetypeMoveIndex == -1ull) { _entitiesToMove.push_back(entityID); }

				else if (entity.archetypeIndex == -1ull)
				{
					_entitiesToMove.push_back(entityID);
					entity.archetypeIndex = ID;
				}

				newArchetype->_entitiesToAdd.push_back(entityID);

				newArchetype->ResizeAddComponentsForNewEntity();

				if (entity.moveComponentIndex != -1ull)
				{
					Archetype* oldArchetype = _archetypeLookup[oldArchetypeMoveIndex];

					// Add data from old archetype to new one
					for (const auto& componentID: oldArchetype->ComponentIDs)
					{
						std::vector<std::byte>& bytes         = newArchetype->_componentDataToAdd[componentID];
						const size_t&           componentSize = _sparseComponentLookup[componentID].Size;
						std::byte*              it            = oldArchetype->_componentDataToAdd[componentID].data() + (entity.moveComponentIndex * componentSize);

						std::move(std::make_move_iterator(it), std::make_move_iterator(it + componentSize), bytes.end() - componentSize);
					}

					// Remove old data form add arrays
					oldArchetype->DestroyEntityInAddQueueImmediately(entityID, false);
				}

				// Add new components to new archetype
				([&]
				{
					std::vector<std::byte>& bytes = newArchetype->GetComponentsToAddRaw<TArgs>();
					TArgs*                  comp  = reinterpret_cast<TArgs*>(bytes.data() + (bytes.size() - sizeof(newComponentData)));
					*comp                         = std::forward<TArgs>(newComponentData);
				}(), ...);

				entity.moveComponentIndex = newArchetype->_entitiesToAdd.size() - 1;
			}

		protected:
			// Destroying
			std::vector<uint64_t> _entitiesToDestroy{};

			// Moving
			std::vector<uint64_t> _entitiesToMove{};

			// Adding
			std::vector<uint64_t>               _entitiesToAdd{};
			std::vector<std::vector<std::byte>> _componentDataToAdd{};

			// Graph variables
			std::vector<uint64_t> _sparseAddComponentArchetypes{};
			std::vector<uint64_t> _sparseRemoveComponentArchetypes{};

			void DestroyQueuedEntities();

		private:
			std::vector<Entity>&      _sparseEntityLookup;
			std::vector<Component>&   _sparseComponentLookup;
			std::vector<Archetype*>&  _archetypeLookup;
			AvailableStack<uint64_t>& _entityGraveyard;

			template<typename T>
			inline std::vector<std::byte>& GetComponentsToAddRaw() { return _componentDataToAdd[TypeIDGenerator<Component>::GetID<T>()]; }

			template<typename... TArgs>
			inline void AddComponents(TArgs&&... components)
			{
				([&]
				{
					std::vector<std::byte>& bytes   = GetComponentsToAddRaw<TArgs>();
					size_t                  oldSize = bytes.size();
					bytes.resize(oldSize + sizeof(components));
					TArgs* comp = reinterpret_cast<TArgs*>(bytes.data() + oldSize);
					*comp       = std::forward<TArgs>(components);
				}(), ...);
			}

			void AddQueuedEntities();

			void MoveQueuedEntities();

			void DestroyEntityImmediately(uint64_t entityID, bool callComponentDestructor);

			void DestroyEntityInAddQueueImmediately(uint64_t entityID, bool callComponentDestructor);

			void ResizeAddComponentsForNewEntity();

			void Resize();
	};
}
