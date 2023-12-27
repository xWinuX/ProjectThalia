#include "SplitEngine/ECS/Registry.hpp"

namespace SplitEngine::ECS
{
	void Registry::Update(float deltaTime)
	{

		for (auto& system : _systems) { system->RunUpdate(this, deltaTime); }
	}

	std::vector<ArchetypeBase*> Registry::GetArchetypesWithSignature(const DynamicBitSet& signature)
	{
		std::vector<ArchetypeBase*> archetypes {};

		for (ArchetypeBase* archetype : ArchetypeBase::_archetypes)
		{
			if (signature.FuzzyMatches(archetype->Signature)) { archetypes.push_back(archetype); }
		}

		return archetypes;
	}

	void Registry::DestroyEntity(uint64_t entityID)
	{
		ArchetypeBase::_archetypes[_sparseEntityLookup[entityID].archetypeIndex]->DestroyEntity(entityID);

		_entityGraveyard.Push(entityID);
	}
}