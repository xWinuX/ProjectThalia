#pragma once

#include <cstdint>

namespace SplitEngine::ECS
{
	struct Entity
	{
		public:
			uint64_t archetypeIndex;
			uint64_t componentIndex;
	};
}