#include "SplitEngine/ECS/ArchetypeBase.hpp"

namespace SplitEngine::ECS
{
	std::vector<ArchetypeBase*> ArchetypeBase::_archetypes = std::vector<ArchetypeBase*>();
	uint64_t                    ArchetypeBase::_id         = 0;

	void ArchetypeBase::DestroyEntity(uint64_t entityID) { RunDestroyEntity(entityID); }
}