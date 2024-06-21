#pragma once
#include "ECS/Archetype.hpp"
#include "ECS/Registry.hpp"

namespace SplitEngine
{
	struct  ECSSettings
	{
		bool                         RootExecutionExecutePendingOperations = true;
		ECS::Registry::ListBehaviour RootExecutionListBehaviour            = ECS::Registry::ListBehaviour::Exclusion;
		std::vector<uint8_t>         RootExecutionStages                   = std::vector<uint8_t>();
	};
}
