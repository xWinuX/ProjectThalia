#pragma once
#include <cstdint>

namespace SplitEngine::ECS
{
	struct Entity
	{
		public:
			uint64_t archetypeIndex     = -1ull;
			uint64_t componentIndex     = -1ull;
			uint64_t moveArchetypeIndex = -1ull;
			uint64_t moveComponentIndex = -1ull;
			uint8_t  group              = -1u;
			uint64_t groupIndex         = -1ull;

			[[nodiscard]] uint64_t GetArchetypeIndex() const { return archetypeIndex != -1ull ? archetypeIndex : moveArchetypeIndex; }

			[[nodiscard]] uint64_t GetComponentIndex() const { return componentIndex != -1ull ? componentIndex : moveComponentIndex; }
	};
}
