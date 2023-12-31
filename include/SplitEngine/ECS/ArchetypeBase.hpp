#pragma once

#include "Component.hpp"
#include "SplitEngine/DataStructures.hpp"

#include <iterator>
#include <vector>

namespace SplitEngine::ECS
{
	class ArchetypeBase
	{
			friend class Registry;

		public:
			uint64_t                            ID = 0;
			std::vector<uint64_t>               Entities {};
			std::vector<std::vector<std::byte>> ComponentData {};
			DynamicBitSet                       Signature {};

			template<typename T>
			std::vector<std::byte>& GetComponentsRaw()
			{
				return ComponentData[TypeIDGenerator<Component>::GetID<T>()];
			}

			template<class T>
			T* GetComponents()
			{
				return reinterpret_cast<T*>(ComponentData[TypeIDGenerator<Component>::GetID<T>()].data());
			}

			template<typename... T>
			uint64_t AddEntity(uint64_t entityID, T&&... args)
			{
				Entities.push_back(entityID);

				((std::copy(reinterpret_cast<const std::byte*>(&args),
							reinterpret_cast<const std::byte*>(&args) + sizeof(args),
							std::back_inserter(GetComponentsRaw<T>()))),
				 ...);

				return Entities.size() - 1;
			}

			void DestroyEntity(uint64_t entityID);

		protected:
			static uint64_t                    _id;
			static std::vector<ArchetypeBase*> _archetypes;

			virtual void RunDestroyEntity(uint64_t entityID) = 0;
	};
}
