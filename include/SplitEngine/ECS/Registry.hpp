#pragma once

#include "SplitEngine/DataStructures.hpp"

#include "Archetype.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "SystemBase.hpp"

#include <iterator>
#include <vector>

namespace SplitEngine::ECS
{
	class Registry
	{
		public:
			Registry() = default;

			void Update(float deltaTime);

			template<typename... T>
			size_t CreateEntity(T&&... args)
			{
				Archetype<T...>& archetype = GetArchetype<T...>();

				uint64_t entityID = 0;
				if (!_entityGraveyard.IsEmpty())
				{
					entityID                                     = _entityGraveyard.Pop();
					uint64_t componentIndex                      = archetype.AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup[entityID].archetypeIndex = archetype.ID;
					_sparseEntityLookup[entityID].componentIndex = componentIndex;
				}
				else
				{
					entityID                = _sparseEntityLookup.size();
					uint64_t componentIndex = archetype.AddEntity(entityID, std::forward<T>(args)...);
					_sparseEntityLookup.emplace_back(archetype.ID, componentIndex);
				}

				return entityID;
			}

			void DestroyEntity(uint64_t entityID);

			template<typename T>
			void RegisterComponent()
			{
				TypeIDGenerator<Component>::GetID<T>();
			}

			template<typename T, typename... TArgs>
			void RegisterSystem(TArgs&&... args)
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				_systems.emplace_back(new T(std::forward<TArgs>(args)...));
			}

			[[nodiscard]] static std::vector<ArchetypeBase*> GetArchetypesWithSignature(const DynamicBitSet& signature);

			template<typename... T>
			Archetype<T...>& GetArchetype()
			{
				static Archetype<T...> archetype = Archetype<T...>(_sparseEntityLookup);
				return archetype;
			}
			
		private:
			std::vector<Entity> _sparseEntityLookup;

			AvailableStack<uint64_t> _entityGraveyard;

			std::vector<SystemBase*> _systems;
	};


}
