#include "SplitEngine/ECS/Archetype.hpp"

namespace SplitEngine::ECS
{
	Archetype::Archetype(std::vector<Entity>&      sparseEntityLookup,
	                     std::vector<Component>&   sparseComponentLookup,
	                     std::vector<Archetype*>&  archetypeLookup,
	                     AvailableStack<uint64_t>& entityGraveyard,
	                     std::vector<uint64_t>&&   componentIDs) :
		ComponentIDs(std::move(componentIDs)),
		_sparseEntityLookup(sparseEntityLookup),
		_sparseComponentLookup(sparseComponentLookup),
		_archetypeLookup(archetypeLookup),
		_entityGraveyard(entityGraveyard)
	{
		// Sort components IDs from lowest to highest
		std::ranges::sort(ComponentIDs);

		Resize();

		ID = _archetypeLookup.size();

		_archetypeLookup.push_back(this);
	}

	void Archetype::DestroyEntity(uint64_t entityID) { _entitiesToDestroy.push_back(entityID); }

	void Archetype::DestroyEntityImmediately(uint64_t entityID, bool callComponentDestructor)
	{
		const size_t indexToRemove = _sparseEntityLookup[entityID].componentIndex;
		const size_t lastIndex     = Entities.size() - 1;

		Entity& lastEntity = _sparseEntityLookup[Entities[lastIndex]];

		if (lastIndex != indexToRemove)
		{
			lastEntity.componentIndex = indexToRemove;

			std::swap(Entities[indexToRemove], Entities[lastIndex]);
		}

		Entities.pop_back();
		for (const uint64_t& ComponentID: ComponentIDs)
		{
			std::vector<std::byte>& bytes                   = ComponentData[ComponentID];
			Component&              component               = _sparseComponentLookup[ComponentID];
			const size_t&           componentSize           = component.Size;
			const size_t            indexToRemoveWithOffset = indexToRemove * componentSize;
			std::byte*              start                   = bytes.data() + indexToRemoveWithOffset;

			if (callComponentDestructor) { component.Destructor(start); }
			if (lastIndex != indexToRemove) { std::swap_ranges(start, start + componentSize, bytes.data() + (lastIndex * componentSize)); }
			bytes.erase(bytes.end() - componentSize, bytes.end());
		}
	}

	void Archetype::DestroyEntityInAddQueueImmediately(uint64_t entityID, bool callComponentDestructor)
	{
		const size_t indexToRemove = _sparseEntityLookup[entityID].moveComponentIndex;
		const size_t lastIndex     = _entitiesToAdd.size() - 1;

		Entity& lastEntity = _sparseEntityLookup[_entitiesToAdd[lastIndex]];
		if (lastIndex != indexToRemove)
		{
			if (lastEntity.moveComponentIndex == -1ull) { lastEntity.componentIndex = indexToRemove; }
			else { lastEntity.moveComponentIndex = indexToRemove; }

			std::swap(_entitiesToAdd[indexToRemove], _entitiesToAdd[lastIndex]);
		}

		_entitiesToAdd.pop_back();
		for (const uint64_t& ComponentID: ComponentIDs)
		{
			std::vector<std::byte>& bytes                   = _componentDataToAdd[ComponentID];
			Component&              component               = _sparseComponentLookup[ComponentID];
			const size_t&           componentSize           = component.Size;
			const size_t            indexToRemoveWithOffset = indexToRemove * componentSize;
			std::byte*              start                   = bytes.data() + indexToRemoveWithOffset;

			if (callComponentDestructor) { component.Destructor(start); }
			if (lastIndex != indexToRemove) { std::swap_ranges(start, start + componentSize, bytes.data() + (lastIndex * componentSize)); }
			bytes.erase(bytes.end() - componentSize, bytes.end());
		}
	}

	void Archetype::AddQueuedEntities()
	{
		if (!_entitiesToAdd.empty())
		{
			Entities.insert(Entities.end(), std::make_move_iterator(_entitiesToAdd.begin()), std::make_move_iterator(_entitiesToAdd.end()));

			for (const auto& indicesToAdd: ComponentIDs)
			{
				std::vector<std::byte>& fromVector = _componentDataToAdd[indicesToAdd];

				std::vector<std::byte>& toVector = ComponentData[indicesToAdd];
				toVector.insert(toVector.end(), std::make_move_iterator(fromVector.begin()), std::make_move_iterator(fromVector.end()));
				fromVector.clear();
			}
		}

		for (const auto& entityID: _entitiesToAdd)
		{
			Entity& entity = _sparseEntityLookup[entityID];

			entity.archetypeIndex     = entity.moveArchetypeIndex;
			entity.componentIndex     = entity.moveComponentIndex;
			entity.moveArchetypeIndex = -1;
			entity.moveComponentIndex = -1;
		}

		_entitiesToAdd.clear();
	}

	void Archetype::MoveQueuedEntities()
	{
		for (uint64_t entityID: _entitiesToMove)
		{
			Entity&    entity    = _sparseEntityLookup[entityID];
			Archetype* archetype = _archetypeLookup[entity.moveArchetypeIndex];

			for (const auto& componentID: archetype->ComponentIDs)
			{
				std::vector<std::byte>& componentData = ComponentData[componentID];
				if (!componentData.empty())
				{
					std::vector<std::byte>& bytes         = archetype->_componentDataToAdd[componentID];
					const size_t&           componentSize = _sparseComponentLookup[componentID].Size;
					std::byte*              it            = componentData.data() + (entity.componentIndex * componentSize);
					if (entity.moveComponentIndex == -1ull) { bytes.insert(bytes.end(), std::make_move_iterator(it), std::make_move_iterator(it + componentSize)); }
					else { std::move(std::make_move_iterator(it), std::make_move_iterator(it + componentSize), bytes.begin() + (entity.moveComponentIndex * componentSize)); }
				}
			}

			if (entity.moveComponentIndex == -1ull)
			{
				archetype->_entitiesToAdd.push_back(entityID);
				entity.moveComponentIndex = (archetype->Entities.size() + archetype->_entitiesToAdd.size()) - 1;
			}

			// Destroy the remains of the moved entity
			if (entity.componentIndex != -1ull) { DestroyEntityImmediately(entityID, false); }
		}

		_entitiesToMove.clear();
	}

	void Archetype::DestroyQueuedEntities()
	{
		for (const uint64_t entityID: _entitiesToDestroy)
		{
			DestroyEntityImmediately(entityID, true);
			_sparseEntityLookup[entityID] = {};
			_entityGraveyard.Push(entityID);
		}

		_entitiesToDestroy.clear();
	}

	void Archetype::Resize()
	{
		const uint64_t numUniqueComponents = TypeIDGenerator<Component>::GetCount();
		Signature.ExtendSizeTo(numUniqueComponents);
		ComponentData.resize(numUniqueComponents);
		_componentDataToAdd.resize(numUniqueComponents);

		_sparseAddComponentArchetypes    = std::vector<uint64_t>(numUniqueComponents, -1);
		_sparseRemoveComponentArchetypes = std::vector<uint64_t>(numUniqueComponents, -1);

		for (const auto& id: ComponentIDs) { Signature.SetBit(id); }
	}

	void Archetype::ResizeAddComponentsForNewEntity()
	{
		for (const auto& componentId: ComponentIDs) { _componentDataToAdd[componentId].resize(_componentDataToAdd[componentId].size() + _sparseComponentLookup[componentId].Size); }
	}
}
