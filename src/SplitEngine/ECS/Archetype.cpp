#include "SplitEngine/ECS/Archetype.hpp"

#define DESTROY_ENTITY_IMPL(entityVector, componentVector)                               \
	entityVector.pop_back();                                                             \
                                                                                         \
	for (uint64_t & ComponentID : ComponentIDs)                                          \
	{                                                                                    \
		std::vector<std::byte>& bytes                   = componentVector[ComponentID];  \
		size_t&                 componentSize           = _componentSizes[ComponentID];  \
		size_t                  indexToRemoveWithOffset = indexToRemove * componentSize; \
                                                                                         \
		if (lastIndex != indexToRemove)                                                  \
		{                                                                                \
			std::swap_ranges(bytes.data() + indexToRemoveWithOffset,                     \
							 bytes.data() + indexToRemoveWithOffset + componentSize,     \
							 bytes.data() + (lastIndex * componentSize));                \
		}                                                                                \
                                                                                         \
		bytes.erase(bytes.end() - componentSize, bytes.end());                           \
	}

namespace SplitEngine::ECS
{
	Archetype::Archetype(std::vector<Entity>&      sparseEntityLookup,
						 std::vector<size_t>&      componentSizes,
						 std::vector<Archetype*>&  archetypeLookup,
						 AvailableStack<uint64_t>& entityGraveyard,
						 std::vector<uint64_t>&&   componentIDs) :
		_sparseEntityLookup(sparseEntityLookup),
		_componentSizes(componentSizes),
		_archetypeLookup(archetypeLookup),
		_entityGraveyard(entityGraveyard),
		ComponentIDs(std::move(componentIDs))
	{
		// Sort components IDs from lowest to highest
		std::sort(ComponentIDs.begin(), ComponentIDs.end());

		Resize();

		ID = _archetypeLookup.size();

		_archetypeLookup.push_back(this);
	}

	void Archetype::DestroyEntity(uint64_t entityID) { _entitiesToDestroy.push_back(entityID); }

	void Archetype::DestroyEntityImmediately(uint64_t entityID)
	{
		size_t indexToRemove = _sparseEntityLookup[entityID].componentIndex;
		size_t lastIndex     = Entities.size() - 1;

		Entity& lastEntity = _sparseEntityLookup[Entities[lastIndex]];

		if (lastIndex != indexToRemove)
		{
			lastEntity.componentIndex = indexToRemove;

			std::swap(Entities[indexToRemove], Entities[lastIndex]);
		}

		DESTROY_ENTITY_IMPL(Entities, ComponentData)
	}

	void Archetype::DestroyEntityInAddQueueImmediately(uint64_t entityID)
	{
		size_t indexToRemove = _sparseEntityLookup[entityID].moveComponentIndex;
		size_t lastIndex     = _entitiesToAdd.size() - 1;

		Entity& lastEntity = _sparseEntityLookup[_entitiesToAdd[lastIndex]];
		if (lastIndex != indexToRemove)
		{
			if (lastEntity.moveComponentIndex == -1) { lastEntity.componentIndex = indexToRemove; }
			else { lastEntity.moveComponentIndex = indexToRemove; }

			std::swap(_entitiesToAdd[indexToRemove], _entitiesToAdd[lastIndex]);
		}

		DESTROY_ENTITY_IMPL(_entitiesToAdd, _componentDataToAdd)
	}

	void Archetype::AddQueuedEntities()
	{
		if (!_entitiesToAdd.empty())
		{
			Entities.insert(Entities.end(), std::make_move_iterator(_entitiesToAdd.begin()), std::make_move_iterator(_entitiesToAdd.end()));

			for (const auto& indicesToAdd : ComponentIDs)
			{
				std::vector<std::byte>& fromVector = _componentDataToAdd[indicesToAdd];

				std::vector<std::byte>& toVector = ComponentData[indicesToAdd];
				toVector.insert(toVector.end(), std::make_move_iterator(fromVector.begin()), std::make_move_iterator(fromVector.end()));
				fromVector.clear();
			}
		}

		_entitiesToAdd.clear();
	}

	void Archetype::MoveQueuedEntities()
	{
		for (uint64_t entityID : _entitiesToMove)
		{
			Entity&    entity    = _sparseEntityLookup[entityID];
			Archetype* archetype = _archetypeLookup[entity.moveArchetypeIndex];

			for (const auto& componentID : archetype->ComponentIDs)
			{
				std::vector<std::byte>& componentData = ComponentData[componentID];
				if (!componentData.empty())
				{
					std::vector<std::byte>& bytes         = archetype->_componentDataToAdd[componentID];
					size_t&                 componentSize = _componentSizes[componentID];
					std::byte*              it            = componentData.data() + (entity.componentIndex * componentSize);
					if (entity.moveComponentIndex == -1)
					{
						bytes.insert(bytes.end(), std::make_move_iterator(it), std::make_move_iterator(it + componentSize));
					}
					else
					{
						std::move(std::make_move_iterator(it),
								  std::make_move_iterator(it + componentSize),
								  bytes.begin() + (entity.moveComponentIndex * componentSize));
					}
				}
			}

			if (entity.moveComponentIndex == -1)
			{
				archetype->_entitiesToAdd.push_back(entityID);
				entity.moveComponentIndex = (archetype->Entities.size() + archetype->_entitiesToAdd.size()) - 1;
			}

			// Destroy the remains of the moved entity
			DestroyEntityImmediately(entityID);

			entity.archetypeIndex     = archetype->ID;
			entity.componentIndex     = entity.moveComponentIndex;
			entity.moveComponentIndex = -1;
			entity.moveArchetypeIndex = -1;
		}

		_entitiesToMove.clear();
	}

	void Archetype::DestroyQueuedEntities()
	{
		for (const uint64_t entityID : _entitiesToDestroy)
		{
			DestroyEntityImmediately(entityID);
			_entityGraveyard.Push(entityID);
		}

		_entitiesToDestroy.clear();
	}

	void Archetype::Resize()
	{
		uint64_t numUniqueComponents = TypeIDGenerator<Component>::GetCount();
		Signature.ExtendSizeTo(numUniqueComponents);
		ComponentData.resize(numUniqueComponents);
		_componentDataToAdd.resize(numUniqueComponents);

		_sparseAddComponentArchetypes    = std::vector<uint64_t>(numUniqueComponents, -1);
		_sparseRemoveComponentArchetypes = std::vector<uint64_t>(numUniqueComponents, -1);

		for (const auto& id : ComponentIDs) { Signature.SetBit(id); }
	}

	void Archetype::ResizeAddComponentsForNewEntity()
	{
		for (const auto& componentId : ComponentIDs)
		{
			_componentDataToAdd[componentId].resize(_componentDataToAdd[componentId].size() + _componentSizes[componentId]);
		}
	}
}
