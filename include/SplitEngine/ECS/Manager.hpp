#pragma once

namespace SplitEngine::ECS
{
	class Manager
	{
		/*
		 * struct Archetype
		 * {
		 * 		std::vector<entityId> entities;
		 * 		std::byte* componentData { packed data (ex.  transform,rigidbody,renderer,transform,rigidbody,renderer...) }
		 * 		size_t archetypeSize; // contains the size in bytes of all components in an archetype (in this case sizeof(transform + rigidbody + renderer))
		 * }
		 *
		 * std::vector<Archetype>
		 *
		 *
		 * Systems have a component signature that tells them what archetypes to look for
		 * the system will search the archetype array for its signature and will loop trough each archetype that has that signature
		 *
		 * Problems to solve:
		 * - How will archetype be generated
		 * - How will archetype searched work? (how to search for a type combination)
		 */
	};
}
