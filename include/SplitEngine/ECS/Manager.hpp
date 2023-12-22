#pragma once

#include <bitset>
#include <unordered_map>
#include <vector>
#include <typeinfo>

namespace SplitEngine::ECS
{
	template<size_t NumComponents>
	class Manager
	{
		public:
			struct Archetype
			{
					size_t                     ID = -1;
					std::vector<size_t>        Entities{};
					std::vector<std::byte>     ComponentData{};
					size_t                     EntitySize = 0;
					std::bitset<NumComponents> Signature {};
			};

			template<typename ...T>
			size_t CreateEntity(T...)
			{

				return _entityID++;
			}

			template<typename T>
			void RegisterComponent()
			{
				_components[(size_t)typeid(T)] = _componentID++;
			}

			template<typename T>
			void RegisterSystem()
			{}

			template<typename... T>
			std::vector<Archetype*> GetArchetypesWithSignature()
			{
				std::vector<Archetype*> archetypes {};
				return archetypes;
			}


		private:
			size_t _componentID = 0;
			size_t _entityID = 0;

			std::vector<Archetype> _archetypes;

			std::vector<std::bitset<NumComponents>> _archetypeSignatures;


			std::unordered_map<size_t, size_t> _components = std::unordered_map<size_t, size_t>();

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
