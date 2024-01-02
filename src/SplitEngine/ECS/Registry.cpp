#include "SplitEngine/ECS/Registry.hpp"

#ifndef SE_HEADLESS
#include "SplitEngine/Rendering/Vulkan/Context.hpp"
#endif

namespace SplitEngine::ECS
{
	Registry::Registry() { _context.Registry = this; }

	Registry::~Registry()
	{
		LOG("Shutting down ECS...");
#ifndef SE_HEADLESS
		for (const SystemBase* system : _renderSystems) { delete system; }
#endif
		for (const SystemBase* system : _gameplaySystems) { delete system; }
	}

	void Registry::Update(float deltaTime)
	{
		_context.DeltaTime = deltaTime;
		for (auto& system : _gameplaySystems) { system->RunExecute(_context); }
	}

	void Registry::Render(float deltaTime)
	{
#ifndef SE_HEADLESS
		_context.DeltaTime = deltaTime;
		for (auto& system : _renderSystems) { system->RunExecute(_context); }
#endif
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