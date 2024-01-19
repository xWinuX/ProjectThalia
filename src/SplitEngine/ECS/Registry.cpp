#include "SplitEngine/ECS/Registry.hpp"

#ifndef SE_HEADLESS
	#include "SplitEngine/Rendering/Vulkan/Context.hpp"
	#include "SplitEngine/Debug/Performance.hpp"
#endif

namespace SplitEngine::ECS
{
	Registry::Registry()
	{
		_context.Registry = this;
		_archetypeRoot    = new Archetype(_sparseEntityLookup, _componentSizes, _archetypeLookup, _entityGraveyard, {});
		_systems          = std::vector<std::vector<SystemBase*>>(static_cast<uint8_t>(Stage::MAX_VALUE), std::vector<SystemBase*>());
	}

	Registry::~Registry()
	{
		LOG("Shutting down ECS...");

		for (const std::vector<SystemBase*>& systems : _systems)
		{
			for (const SystemBase* system : systems) { delete system; }
		}

		for (const Archetype* archetype : _archetypeLookup) { delete archetype; }
	}

	void Registry::PrepareForExecution(float deltaTime)
	{
		MoveQueuedEntities();
		AddQueuedEntities();
		DestroyQueuedEntities();

		_context.DeltaTime = deltaTime;
	}

	void Registry::AddQueuedEntities()
	{
		for (const auto& archetype : _archetypeLookup) { archetype->AddQueuedEntities(); }
	}

	void Registry::DestroyQueuedEntities()
	{
		for (const auto& archetype : _archetypeLookup) { archetype->DestroyQueuedEntities(); }
	}

	void Registry::MoveQueuedEntities()
	{
		for (const auto& archetype : _archetypeLookup) { archetype->MoveQueuedEntities(); }
	}

	std::vector<Archetype*> Registry::GetArchetypesWithSignature(const DynamicBitSet& signature)
	{
		std::vector<Archetype*> archetypes {};

		for (Archetype* archetype : _archetypeLookup)
		{
			if (signature.FuzzyMatches(archetype->Signature)) { archetypes.push_back(archetype); }
		}

		return archetypes;
	}

	void Registry::DestroyEntity(uint64_t entityID) { _archetypeLookup[_sparseEntityLookup[entityID].archetypeIndex]->DestroyEntity(entityID); }

	void Registry::RegisterAssetDatabase(SplitEngine::AssetDatabase* assetDatabase) { _context.AssetDatabase = assetDatabase; }

	bool Registry::IsEntityValid(uint64_t entityID)
	{
		Entity& entity = _sparseEntityLookup[entityID];
		return entity.GetArchetypeIndex() != -1;
	}

	void Registry::ExecuteSystems(Stage stageToExecute)
	{
		for (const auto& system : _systems[static_cast<uint8_t>(stageToExecute)]) { system->RunExecute(_context); }
	}

}