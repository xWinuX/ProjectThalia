#include "SplitEngine/ECS/Archetype.hpp"

#define DESTROY_ENTITY_IMPL(entityVector, componentVector, componentIndexVar)            \
	size_t indexToRemove = _sparseEntityLookup[entityID].componentIndexVar;                 \
	size_t lastIndex     = entityVector.size() - 1;                                      \
                                                                                         \
	if (lastIndex != indexToRemove)                                                      \
	{                                                                                    \
		_sparseEntityLookup[entityVector[lastIndex]].componentIndex = indexToRemove;     \
		std::swap(entityVector[indexToRemove], entityVector[lastIndex]);                 \
	}                                                                                    \
                                                                                         \
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
	std::vector<Archetype*> Archetype::_archetypes = std::vector<Archetype*>();
	uint64_t                Archetype::_id         = 0;

	Archetype::Archetype(std::vector<Entity>& sparseEntityLookup, std::vector<size_t>& componentSizes, std::vector<uint64_t>&& componentIDs) :
		_sparseEntityLookup(sparseEntityLookup),
		_componentSizes(componentSizes),
		ComponentIDs(std::move(componentIDs))
	{
		// Sort components IDs from lowest to highest
		std::sort(ComponentIDs.begin(), ComponentIDs.end());

		Resize();

		ID = _id++;

		_archetypes.push_back(this);
	}

	void Archetype::DestroyEntity(uint64_t entityID) { _entitiesToDestroy.push_back(entityID); }

	void Archetype::DestroyQueuedEntities()
	{
		for (const uint64_t entityID : _entitiesToDestroy)
		{
			size_t indexToRemove = _sparseEntityLookup[entityID].componentIndex;
			size_t lastIndex     = Entities.size() - 1;

			if (lastIndex != indexToRemove)
			{
				_sparseEntityLookup[Entities[lastIndex]].componentIndex = indexToRemove;
				std::swap(Entities[indexToRemove], Entities[lastIndex]);
			}

			Entities.pop_back();

			for (uint64_t& ComponentID : ComponentIDs)
			{
				std::vector<std::byte>& bytes                   = ComponentData[ComponentID];
				size_t&                 componentSize           = _componentSizes[ComponentID];
				size_t                  indexToRemoveWithOffset = indexToRemove * componentSize;

				if (lastIndex != indexToRemove)
				{
					std::swap_ranges(bytes.data() + indexToRemoveWithOffset,
									 bytes.data() + indexToRemoveWithOffset + componentSize,
									 bytes.data() + (lastIndex * componentSize));
				}

				bytes.erase(bytes.end() - componentSize, bytes.end());
			}
		}

		_entitiesToDestroy.clear();
	}

	void Archetype::DestroyEntityImmediately(uint64_t entityID) { DESTROY_ENTITY_IMPL(Entities, ComponentData, componentIndex) }

	void Archetype::DestroyEntityInAddQueueImmediately(uint64_t entityID) { DESTROY_ENTITY_IMPL(_entitiesToAdd, _componentDataToAdd, moveComponentIndex) }

	void Archetype::AddQueuedEntities()
	{
		Entities.insert(Entities.end(), std::make_move_iterator(_entitiesToAdd.begin()), std::make_move_iterator(_entitiesToAdd.end()));
		_entitiesToAdd.clear();

		for (const auto& indicesToAdd : ComponentIDs)
		{
			std::vector<std::byte>& fromVector = _componentDataToAdd[indicesToAdd];

			std::vector<std::byte>& toVector = ComponentData[indicesToAdd];
			toVector.insert(toVector.end(), std::make_move_iterator(fromVector.begin()), std::make_move_iterator(fromVector.end()));
			fromVector.clear();
		}
	}

	void Archetype::MoveQueuedEntities()
	{
		for (uint64_t entityID : _entitiesToMove)
		{
			Entity&    entity    = _sparseEntityLookup[entityID];
			Archetype* archetype = _archetypes[entity.moveArchetypeIndex];

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