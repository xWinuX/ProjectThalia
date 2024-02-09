#include "SplitEngine/ECS/Registry.hpp"

#ifndef SE_HEADLESS
#include "SplitEngine/Debug/Performance.hpp"
#include "SplitEngine/Rendering/Vulkan/Instance.hpp"
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

		for (const std::vector<SystemEntry>& systems: _systems) { for (const SystemEntry& systemEntry: systems) { delete systemEntry.System; } }

		for (const Archetype* archetype: _archetypeLookup) { delete archetype; }
	}

	void Registry::PrepareForExecution(float deltaTime)
	{
		for (const auto& archetype: _archetypeLookup) { archetype->MoveQueuedEntities(); }

		for (const auto& archetype: _archetypeLookup) { archetype->AddQueuedEntities(); }

		for (const auto& archetype: _archetypeLookup) { archetype->DestroyQueuedEntities(); }

		RemoveQueuedSystems();

		AddQueuedSystems();

		_context.DeltaTime = deltaTime;
	}

	void Registry::RemoveQueuedSystems()
	{
		for (const auto& systemsToRemoveID: _systemsToRemove)
		{
			SystemLookupEntry& systemLookupEntry = _sparseSystemLookup[systemsToRemoveID];

			const uint64_t            stage   = static_cast<uint64_t>(systemLookupEntry.Stage);
			std::vector<SystemEntry>& systems = systemLookupEntry.Added ? _systems[stage] : _systemsToAdd[stage];

			const auto& it = systems.begin() + systemLookupEntry.Index;
			delete it->System;
			systems.erase(it);

			for (uint64_t i = systemLookupEntry.Index; i < systems.size(); ++i) { --_sparseSystemLookup[systems[i].ID].Index; }

			systemLookupEntry.Index = -1;
			systemLookupEntry.Added = false;

			_systemGraveyard.Push(systemsToRemoveID);
		}

		_systemsToRemove.clear();
	}

	void Registry::AddQueuedSystems()
	{
		for (int stage = 0; stage < static_cast<uint32_t>(Stage::MAX_VALUE); ++stage)
		{
			for (const auto& insertSystemEntry: _systemsToAdd[stage])
			{
				std::vector<SystemEntry>& systems                 = _systems[stage];
				SystemLookupEntry&        insertSystemLookupEntry = _sparseSystemLookup[insertSystemEntry.ID];
				insertSystemLookupEntry.Added                     = true;

				// Just insert if vector is empty
				if (systems.empty())
				{
					systems.push_back(insertSystemEntry);
					insertSystemLookupEntry.Index = 0;
					continue;
				}

				// Insert at back if order is biggest
				if (insertSystemEntry.Order > systems.back().Order)
				{
					systems.insert(systems.end(), insertSystemEntry);
					insertSystemLookupEntry.Index = systems.size() - 1;
					continue;
				}

				// Search for correct placement
				bool found = false;
				for (int systemIndex = 0; systemIndex < systems.size(); ++systemIndex)
				{
					SystemEntry& systemEntry = systems[systemIndex];

					if (found) { ++_sparseSystemLookup[systemEntry.ID].Index; }
					else
					{
						if (insertSystemEntry.Order <= systemEntry.Order)
						{
							systems.insert(systems.begin() + systemIndex, insertSystemEntry);
							insertSystemLookupEntry.Index = systemIndex;
							found                         = true;
						}
					}
				}
			}

			_systemsToAdd[stage].clear();
		}
	}

	std::vector<Archetype*> Registry::GetArchetypesWithSignature(const DynamicBitSet& signature)
	{
		std::vector<Archetype*> archetypes{};

		for (Archetype* archetype: _archetypeLookup) { if (signature.FuzzyMatches(archetype->Signature)) { archetypes.push_back(archetype); } }

		return archetypes;
	}

	void Registry::DestroyEntity(uint64_t entityID) { _archetypeLookup[_sparseEntityLookup[entityID].archetypeIndex]->DestroyEntity(entityID); }

	void Registry::RegisterAssetDatabase(SplitEngine::AssetDatabase* assetDatabase) { _context.AssetDatabase = assetDatabase; }

	bool Registry::IsEntityValid(uint64_t entityID)
	{
		Entity& entity = _sparseEntityLookup[entityID];
		return entity.GetArchetypeIndex() != -1;
	}

	bool Registry::IsSystemValid(uint64_t systemID)
	{
		SystemLookupEntry& systemLookupEntry = _sparseSystemLookup[systemID];
		return systemLookupEntry.Index != -1;
	}

	void Registry::ExecuteSystems(Stage stageToExecute)
	{
		for (const auto& systemEntry: _systems[static_cast<uint8_t>(stageToExecute)]) { systemEntry.System->RunExecute(_context); }
	}

	void Registry::RemoveSystem(uint64_t systemID) { _systemsToRemove.push_back(systemID); }
}
