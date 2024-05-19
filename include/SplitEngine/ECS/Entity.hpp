#pragma once
#include <cstdint>

namespace SplitEngine::ECS
{
	struct Entity
	{
		public:
			uint64_t archetypeIndex     = -1;
			uint64_t componentIndex     = -1;
			uint64_t moveArchetypeIndex = -1;
			uint64_t moveComponentIndex = -1;

			[[nodiscard]] uint64_t GetArchetypeIndex() const { return archetypeIndex != -1 ? archetypeIndex : moveArchetypeIndex; }

			[[nodiscard]] uint64_t GetComponentIndex() const { return componentIndex != -1 ? componentIndex : moveComponentIndex; }
	};
}
