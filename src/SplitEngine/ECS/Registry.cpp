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
	}

	Registry::~Registry()
	{
		LOG("Shutting down ECS...");
#ifndef SE_HEADLESS
		for (const SystemBase* system : _renderSystems) { delete system; }
#endif
		for (const SystemBase* system : _gameplaySystems) { delete system; }

		for (const Archetype* archetype : _archetypeLookup) { delete archetype; }
	}

	void Registry::Update(float deltaTime)
	{
		MoveQueuedEntities();
		AddQueuedEntities();
		DestroyQueuedEntities();

		_context.DeltaTime = deltaTime;
		for (auto& system : _gameplaySystems) { system->RunExecute(_context); }
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

	void Registry::Render(float deltaTime)
	{
#ifndef SE_HEADLESS
		_context.DeltaTime = deltaTime;
		for (auto& system : _renderSystems) { system->RunExecute(_context); }
#endif
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

	void Registry::DestroyEntity(uint64_t entityID)
	{
		_archetypeLookup[_sparseEntityLookup[entityID].archetypeIndex]->DestroyEntity(entityID);

	}

	void Registry::RegisterAssetDatabase(SplitEngine::AssetDatabase* assetDatabase) { _context.AssetDatabase = assetDatabase; }
}