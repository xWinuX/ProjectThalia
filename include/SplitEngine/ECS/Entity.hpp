#pragma once

#include <cstdint>

namespace SplitEngine::ECS
{
	struct Entity
	{
		public:
			uint64_t archetypeIndex     = -1;
			uint64_t componentIndex     = -1;
			uint64_t moveComponentIndex = -1;
			uint64_t moveArchetypeIndex = -1;
	};
}