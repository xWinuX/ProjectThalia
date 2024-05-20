#include "SplitEngine/ECS/Registry.hpp"

#include <SDL_timer.h>

namespace SplitEngine::ECS
{
	Registry::Registry()
	{
		_contextProvider.Registry = this;
		_archetypeRoot            = new Archetype(_sparseEntityLookup, _sparseComponentLookup, _archetypeLookup, _entityGraveyard, {});
	}

	Registry::~Registry()
	{
		LOG("Shutting down ECS...");

		for (SystemEntry& system: _systems)
		{
			system.System->Destroy(_contextProvider);
			delete system.System;
		}

		for (const Archetype* archetype: _archetypeLookup) { delete archetype; }
	}

	void Registry::PrepareForExecution()
	{
		for (const auto& archetype: _archetypeLookup) { archetype->MoveQueuedEntities(); }

		for (const auto& archetype: _archetypeLookup) { archetype->AddQueuedEntities(); }

		for (const auto& archetype: _archetypeLookup) { archetype->DestroyQueuedEntities(); }

		RemoveQueuedSystems();

		AddQueuedSystems();

		// Reset cached state on system
		for (SystemEntry& system: _systems) { if (IsSystemValid(system.ID)) { system.System->_cachedArchetypes = false; } }
	}

	void Registry::RemoveQueuedSystems()
	{
		for (const auto& systemsToRemoveID: _systemsToRemove)
		{
			SystemEntry& systemEntry = _systems[systemsToRemoveID];
			systemEntry.ID           = -1;
			systemEntry.System->Destroy(_contextProvider);
			delete systemEntry.System;

			if (!systemEntry.Added)
			{
				for (SystemLocation& location: systemEntry.Locations)
				{
					std::vector<SystemExecutionEntry>& systemExecutionEntries = _systemExecutionFlow[location.Stage];
					for (uint64_t i = location.StageIndex; i < systemExecutionEntries.size(); ++i) { --GetSystemLocationOfExecutionEntry(systemExecutionEntries[i]).StageIndex; }

					std::vector<SystemExecutionEntry>::iterator it = systemExecutionEntries.begin();
					std::advance(it, location.StageIndex);
					systemExecutionEntries.erase(it);
				}
			}

			_systemGraveyard.Push(systemsToRemoveID);
		}

		// Deactivate empty stages
		for (uint8_t activeStage: _activeStages) { if (_systemExecutionFlow[activeStage].empty()) { _tmpStagesToDeactivate.push_back(activeStage); } }

		for (uint8_t stageToDeactivate: _tmpStagesToDeactivate) { _activeStages.erase(std::ranges::remove(_activeStages, stageToDeactivate).begin()); }

		_tmpStagesToDeactivate.clear();
		_systemsToRemove.clear();
	}

	Registry::SystemLocation& Registry::GetSystemLocationOfExecutionEntry(const SystemExecutionEntry& executionEntry)
	{
		return _systems[executionEntry.SystemID].Locations[executionEntry.SystemLocationIndex];
	}

	std::vector<float>&   Registry::GetAccumulatedStageTimeMs() { return _accumulatedStageTimeMs; }
	std::vector<uint8_t>& Registry::GetActiveStages() { return _activeStages; }

	void Registry::AddQueuedSystems()
	{
		if (_systems.empty()) { return; }

		for (SystemEntry& system: _systems)
		{
			if (system.Added) { continue; }
			system.Added = true;

			for (uint64_t locationIndex = 0; locationIndex < system.Locations.size(); ++locationIndex)
			{
				SystemLocation& location = system.Locations[locationIndex];
				_tmpActiveStages.insert(location.Stage);

				std::vector<SystemExecutionEntry>& executionEntriesOfCurrentStage = _systemExecutionFlow[location.Stage];
				SystemExecutionEntry               executionEntry                 = { system.ID, locationIndex };

				// Just insert if vector is empty
				if (executionEntriesOfCurrentStage.empty())
				{
					executionEntriesOfCurrentStage.push_back(executionEntry);
					location.StageIndex = 0;
					continue;
				}

				// Insert at back if order is biggest
				SystemLocation& backLocation = GetSystemLocationOfExecutionEntry(executionEntriesOfCurrentStage.back());
				if (location.Order > backLocation.Order)
				{
					executionEntriesOfCurrentStage.insert(executionEntriesOfCurrentStage.end(), executionEntry);
					location.StageIndex = executionEntriesOfCurrentStage.size() - 1;
					continue;
				}

				// Search for correct placement
				bool found = false;
				for (int stageIndex = 0; stageIndex < executionEntriesOfCurrentStage.size(); ++stageIndex)
				{
					SystemExecutionEntry& systemExecutionEntry = executionEntriesOfCurrentStage[stageIndex];

					SystemLocation& currentSystemLocation = GetSystemLocationOfExecutionEntry(systemExecutionEntry);
					if (found) { ++currentSystemLocation.StageIndex; }
					else
					{
						if (location.Order <= currentSystemLocation.Order)
						{
							executionEntriesOfCurrentStage.insert(executionEntriesOfCurrentStage.begin() + stageIndex, executionEntry);
							location.StageIndex = stageIndex;
							found               = true;
						}
					}
				}
			}
		}

		if (!_tmpActiveStages.empty()) { _activeStages = std::vector(_tmpActiveStages.begin(), _tmpActiveStages.end()); }
		_tmpActiveStages.clear();
	}

	std::vector<Archetype*> Registry::GetArchetypesWithSignature(const DynamicBitSet& signature)
	{
		std::vector<Archetype*> archetypes{};

		for (Archetype* archetype: _archetypeLookup) { if (signature.FuzzyMatches(archetype->Signature)) { archetypes.push_back(archetype); } }

		return archetypes;
	}

	ContextProvider& Registry::GetContextProvider() { return _contextProvider; }

	void Registry::DestroyEntity(uint64_t entityID) { _archetypeLookup[_sparseEntityLookup[entityID].archetypeIndex]->DestroyEntity(entityID); }

	bool Registry::IsEntityValid(uint64_t entityID)
	{
		Entity& entity = _sparseEntityLookup[entityID];
		return entity.GetArchetypeIndex() != -1;
	}

	bool Registry::IsSystemValid(uint64_t systemID) const
	{
		if (systemID >= _systems.size()) { return false; }
		return _systems[systemID].ID != -1;
	}

	void Registry::SetEnableStatistics(const bool enabled) { _collectStatistics = enabled; }

	void Registry::ExecuteSystems()
	{
		PrepareForExecution();

		// Run all active stages
		uint64_t stageStartTime = 0;
		uint64_t stageEndTime   = 0;
		for (const uint8_t stage: _activeStages)
		{
			std::vector<SystemExecutionEntry>& systemExecutionEntries = _systemExecutionFlow[stage];

			if (_collectStatistics) { stageStartTime = SDL_GetPerformanceCounter(); }

			for (SystemExecutionEntry& systemExecutionEntry: systemExecutionEntries) { _systems[systemExecutionEntry.SystemID].System->RunExecute(_contextProvider, stage); }

			if (_collectStatistics)
			{
				stageEndTime = SDL_GetPerformanceCounter();
				_accumulatedStageTimeMs[stage] += static_cast<float>((stageEndTime - stageStartTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency());
			}
		}
	}

	void Registry::RemoveSystem(uint64_t systemID) { _systemsToRemove.push_back(systemID); }
}
