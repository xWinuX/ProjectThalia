#pragma once

#include "SplitEngine/DataStructures.hpp"
#include "SplitEngine/Debug/Log.hpp"
#include <bitset>
#include <unordered_map>
#include <vector>

namespace SplitEngine
{
	struct Component
	{};

	template<class T>
	class TypeIDGenerator
	{
		private:
			static uint64_t _count;

		public:
			template<class U>
			static uint64_t GetID()
			{
				static const uint64_t ID = _count++;
				return ID;
			}

			static uint64_t GetCount() { return _count; }
	};

	template<class T>
	uint64_t TypeIDGenerator<T>::_count = 0;

	class ECS
	{
		public:
			ECS() = default;

			class ArchetypeBase
			{
				public:
					uint64_t                                           ID = 0;
					std::vector<uint64_t>                              Entities {};
					std::unordered_map<size_t, std::vector<std::byte>> ComponentData {};
					size_t                                             EntitySize = 0;
					DynamicBitSet                                      Signature {};

					template<typename T>
					std::vector<std::byte>& GetComponents()
					{
						return ComponentData.at(TypeIDGenerator<Component>::GetID<T>());
					}

				protected:
					static uint64_t                    _id;
					static std::vector<ArchetypeBase*> _archetypes;
			};

			template<typename... T>
			class Archetype : public ArchetypeBase
			{
				public:
					explicit Archetype()
					{
						Signature.ExtendSizeBy(TypeIDGenerator<Component>::GetCount());

						(
								[&] {
									uint64_t id = TypeIDGenerator<Component>::GetID<T>();
									Signature.SetBit(id);
									ComponentData.try_emplace(id, std::vector<std::byte>());
								}(),
								...);

						EntitySize = (sizeof(T) + ... + 0);

						ID = _id++;

						_archetypes.push_back(this);
					}
			};

			template<typename... T>
			size_t CreateEntity(T... args)
			{
				++_entityID;

				Archetype<T...>& archetype = GetArchetype<T...>();

				archetype.Entities.push_back(_entityID);

				// Jesus fucking christ
				// Copies data into the archetype
				((std::copy(reinterpret_cast<const std::byte*>(&args),
							reinterpret_cast<const std::byte*>(&args) + sizeof(args),
							std::back_inserter(archetype.template GetComponents<T>()))),
				 ...);

				_entityLocation.try_emplace(_entityID, archetype.ID);

				return _entityID;
			}

			template<typename T>
			void RegisterComponent()
			{
				TypeIDGenerator<Component>::GetID<T>();
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
			uint64_t _componentID = 0;
			uint64_t _entityID    = 0;

			std::unordered_map<uint64_t, size_t> _entityLocation;
	};

	inline std::vector<ECS::ArchetypeBase*>        ECS::ArchetypeBase::_archetypes = std::vector<ECS::ArchetypeBase*>();
	inline uint64_t                                ECS::ArchetypeBase::_id         = 0;
}
