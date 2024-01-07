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
			std::vector<uint64_t>               Entities {};
			std::vector<std::vector<std::byte>> ComponentData {};
			std::vector<uint64_t>               ComponentIDs {};
			DynamicBitSet                       Signature {};

			Archetype(std::vector<Entity>&      sparseEntityLookup,
					  std::vector<size_t>&      componentSizes,
					  std::vector<Archetype*>&  archetypeLookup,
					  AvailableStack<uint64_t>& entityGraveyard,
					  std::vector<uint64_t>&&   componentIDs);

			template<typename T>
			std::vector<std::byte>& GetComponentsRaw()
			{
				return ComponentData[TypeIDGenerator<Component>::GetID<T>()];
			}

			template<class T>
			T* GetComponents()
			{
				return reinterpret_cast<T*>(ComponentData[TypeIDGenerator<Component>::GetID<T>()].data());
			}

			template<typename... T>
			uint64_t AddEntity(uint64_t entityID, T&&... args)
			{
				_entitiesToAdd.push_back(entityID);

				(GetComponentsToAddRaw<T>().insert(GetComponentsToAddRaw<T>().end(),
												   std::make_move_iterator(reinterpret_cast<const std::byte*>(&args)),
												   std::make_move_iterator(reinterpret_cast<const std::byte*>(&args) + sizeof(args))),
				 ...);

				return (Entities.size() + _entitiesToAdd.size()) - 1;
			}

			void DestroyEntity(uint64_t entityID);

			template<typename... T>
			Archetype* FindArchetype()
			{
				if (sizeof...(T) == 0) { return this; }

				uint64_t index = -1;
				(
						[&] {
							if (index != -1) { index = _archetypeLookup[index]->GetAddArchetypeID<T>(); }
							else { index = GetAddArchetypeID<T>(); }
						}(),
						...);

				return _archetypeLookup[index];
			}

			template<typename T>
			uint64_t GetAddArchetypeID()
			{
				uint64_t componentIDToAdd = TypeIDGenerator<Component>::GetID<T>();

				if (_sparseAddComponentArchetypes[componentIDToAdd] == -1)
				{
					std::vector<uint64_t> componentIds = std::vector<uint64_t>(ComponentIDs);
					componentIds.push_back(componentIDToAdd);
					Archetype* archetype = new Archetype(_sparseEntityLookup, _componentSizes, _archetypeLookup, _entityGraveyard, std::move(componentIds));
					_sparseAddComponentArchetypes[componentIDToAdd] = archetype->ID;
					return archetype->ID;
				}
				return _sparseAddComponentArchetypes[componentIDToAdd];
			}

			template<typename T>
			uint64_t GetRemoveArchetypeID()
			{
				uint64_t componentIDToRemove = TypeIDGenerator<Component>::GetID<T>();
				if (_sparseRemoveComponentArchetypes[componentIDToRemove] == -1)
				{
					std::vector<uint64_t> componentIds;
					componentIds.reserve(ComponentIDs.size() - 1);

					for (uint64_t& componentID : ComponentIDs)
					{
						if (componentID != componentIDToRemove) { componentIds.push_back(componentID); }
					}

					Archetype* archetype = new Archetype(_sparseEntityLookup, _componentSizes, _archetypeLookup, _entityGraveyard, std::move(componentIds));
					_sparseRemoveComponentArchetypes[componentIDToRemove]         = archetype->ID;
					archetype->_sparseAddComponentArchetypes[componentIDToRemove] = ID;
					return archetype->ID;
				}
				return _sparseRemoveComponentArchetypes[componentIDToRemove];
			}

			template<typename... T>
			void RemoveComponentsFromEntity(uint64_t entityID)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				uint64_t oldArchetypeMoveIndex = entity.moveArchetypeIndex;

				// Recursively search for archetype in tree
				(
						[&] {
							if (entity.moveArchetypeIndex != -1)
							{
								entity.moveArchetypeIndex = _archetypeLookup[entity.moveArchetypeIndex]->GetRemoveArchetypeID<T>();
							}
							else { entity.moveArchetypeIndex = GetRemoveArchetypeID<T>(); }
						}(),
						...);

				if (oldArchetypeMoveIndex == -1) { _entitiesToMove.push_back(entityID); }
				else if (entity.moveComponentIndex != -1)
				{
					Archetype* newArchetype = _archetypeLookup[entity.moveArchetypeIndex];
					Archetype* oldArchetype = _archetypeLookup[oldArchetypeMoveIndex];

					newArchetype->_entitiesToAdd.push_back(entityID);

					newArchetype->ResizeAddComponentsForNewEntity();

					// Add data from old archetype to new one
					for (const auto& componentID : newArchetype->ComponentIDs)
					{
						std::vector<std::byte>& bytes         = newArchetype->_componentDataToAdd[componentID];
						size_t&                 componentSize = _componentSizes[componentID];
						std::byte*              it = oldArchetype->_componentDataToAdd[componentID].data() + (entity.moveComponentIndex * componentSize);

						std::move(std::make_move_iterator(it), std::make_move_iterator(it + componentSize), bytes.end() - componentSize);
					}

					// Remove old data form add arrays
					oldArchetype->DestroyEntityInAddQueueImmediately(entityID);

					entity.moveComponentIndex = newArchetype->_entitiesToAdd.size() - 1;
				}
			}

			template<typename... T>
			void AddComponentsToEntity(uint64_t entityID, T&&... newComponentData)
			{
				Entity& entity = _sparseEntityLookup[entityID];

				uint64_t oldArchetypeMoveIndex = entity.moveArchetypeIndex;

				// Recursively search for archetype in tree
				(
						[&] {
							if (entity.moveArchetypeIndex != -1)
							{
								entity.moveArchetypeIndex = _archetypeLookup[entity.moveArchetypeIndex]->GetAddArchetypeID<T>();
							}
							else { entity.moveArchetypeIndex = GetAddArchetypeID<T>(); }
						}(),
						...);


				Archetype* newArchetype = _archetypeLookup[entity.moveArchetypeIndex];

				if (oldArchetypeMoveIndex == -1) { _entitiesToMove.push_back(entityID); }

				else if (entity.archetypeIndex == -1)
				{
					_entitiesToMove.push_back(entityID);
					entity.archetypeIndex = ID;
				}

				newArchetype->_entitiesToAdd.push_back(entityID);

				newArchetype->ResizeAddComponentsForNewEntity();

				if (entity.moveComponentIndex != -1)
				{
					Archetype* oldArchetype = _archetypeLookup[oldArchetypeMoveIndex];

					// Add data from old archetype to new one
					for (const auto& componentID : oldArchetype->ComponentIDs)
					{
						std::vector<std::byte>& bytes         = newArchetype->_componentDataToAdd[componentID];
						size_t&                 componentSize = _componentSizes[componentID];
						std::byte*              it = oldArchetype->_componentDataToAdd[componentID].data() + (entity.moveComponentIndex * componentSize);

						std::move(std::make_move_iterator(it), std::make_move_iterator(it + componentSize), bytes.end() - componentSize);
					}

					// Remove old data form add arrays
					oldArchetype->DestroyEntityInAddQueueImmediately(entityID);
				}


				(std::move(std::make_move_iterator(reinterpret_cast<const std::byte*>(&newComponentData)),
						   std::make_move_iterator(reinterpret_cast<const std::byte*>(&newComponentData) + sizeof(newComponentData)),
						   newArchetype->GetComponentsToAddRaw<T>().end() - sizeof(newComponentData)),
				 ...);

				entity.moveComponentIndex = newArchetype->_entitiesToAdd.size() - 1;
			}

		protected:
			// Destroying
			std::vector<uint64_t> _entitiesToDestroy {};

			// Moving
			std::vector<uint64_t> _entitiesToMove {};

			// Adding
			std::vector<uint64_t>               _entitiesToAdd {};
			std::vector<std::vector<std::byte>> _componentDataToAdd {};

			// Graph variables
			std::vector<uint64_t> _sparseAddComponentArchetypes {};
			std::vector<uint64_t> _sparseRemoveComponentArchetypes {};

			void DestroyQueuedEntities();

		private:
			std::vector<Entity>&      _sparseEntityLookup;
			std::vector<size_t>&      _componentSizes;
			std::vector<Archetype*>&  _archetypeLookup;
			AvailableStack<uint64_t>& _entityGraveyard;

			template<typename T>
			std::vector<std::byte>& GetComponentsToAddRaw()
			{
				return _componentDataToAdd[TypeIDGenerator<Component>::GetID<T>()];
			}

			void AddQueuedEntities();
			void MoveQueuedEntities();
			void DestroyEntityImmediately(uint64_t entityID);
			void DestroyEntityInAddQueueImmediately(uint64_t entityID);

			void ResizeAddComponentsForNewEntity();

			void Resize();
	};
}
