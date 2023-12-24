#pragma once

#include "SplitEngine/DataStructures.hpp"
#include "SplitEngine/Debug/Log.hpp"
#include <bitset>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace SplitEngine
{
	class ECSManager
	{
		public:
			ECSManager() = default;

			// TODO: somehow remove this mechanism

			struct ArchetypeBase
			{
					uint64_t               ID = -1;
					std::vector<uint64_t>  Entities {};
					std::vector<std::byte> ComponentData {};
					size_t                 EntitySize = 0;
					DynamicBitSet          Signature {};

					std::function<void()> _apply;

					static std::vector<ArchetypeBase*> _archetypes;
			};

			template<typename... T>
			struct Archetype : ArchetypeBase
			{
					Archetype()
					{
						Signature.ExtendSizeBy(_components.size());

						// TODO: remove this monstrosity
						(
								[&] {
									auto hash = typeid(T).hash_code();
									auto id   = _components.at(hash);
								}(),
								...);


						_archetypes.push_back(this);
					}
			};

			template<typename T>
			void RegisterComponent()
			{
				_components.try_emplace(typeid(T).hash_code(), ++_componentID);
			}

			template<typename... T>
			size_t CreateEntity(T... args)
			{
				++_entityID;

				GetArchetype<T...>().Entities.push_back(_entityID);

				// Jesus fucking christ
				// Copies data into the archetype
				auto destIterator = std::back_inserter(GetArchetype<T...>().ComponentData);
				((std::copy(reinterpret_cast<const std::byte*>(&args), reinterpret_cast<const std::byte*>(&args) + sizeof(args), destIterator)), ...);

				return _entityID;
			}

			template<typename T>
			void RegisterSystem()
			{}

			template<typename... T>
			Archetype<T...>& GetArchetype()
			{
				static Archetype<T...> archetype;
				return archetype;
			}

		private:
			size_t _componentID = 0;
			size_t _entityID    = 0;

			static std::unordered_map<size_t, size_t> _components;

			template<typename... T>
			void AddEntityToArchetype(size_t entityID, const std::vector<std::byte>& componentData)
			{}

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

	inline std::unordered_map<size_t, size_t> ECSManager::_components = std::unordered_map<size_t, size_t>();

	inline std::vector<ECSManager::ArchetypeBase*> ECSManager::ArchetypeBase::_archetypes = std::vector<ECSManager::ArchetypeBase*>();
}
