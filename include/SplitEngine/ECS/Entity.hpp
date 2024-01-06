#pragma once

#include <cstdint>

namespace SplitEngine::ECS
{
	struct Entity
	{
		public:
			uint64_t archetypeIndex;
			uint64_t componentIndex;
			uint64_t moveComponentIndex = -1;
			uint64_t moveArchetypeIndex = -1;
	};
}