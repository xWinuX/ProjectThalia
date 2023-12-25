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

	// Source: https://indiegamedev.net/2020/05/19/an-entity-component-system-with-data-locality-in-cpp/
	template<typename TBase>
	class TypeIDGenerator
	{
		private:
			static uint64_t _count;

		public:
			template<typename T>
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

			void Update(float deltaTime)
			{
				for (auto& system : _systems) { system->RunUpdate(*this, deltaTime); }
			}

			class SystemBase
			{
				public:
					virtual void RunUpdate(const ECS& ecs, float deltaTime) = 0;
			};

			template<typename... T>
			class System : public SystemBase
			{
				public:
					System()
					{
						Signature.ExtendSizeBy(TypeIDGenerator<Component>::GetCount());
						(Signature.SetBit(TypeIDGenerator<Component>::GetID<T>()), ...);
					}

					void RunUpdate(const ECS& ecs, float deltaTime) final
					{
						std::vector<ECS::ArchetypeBase*> archetypes = ecs.GetArchetypesWithSignature(Signature);

						for (ECS::ArchetypeBase* archetype : archetypes)
						{
							std::apply(
									[this, &archetype, deltaTime](T*... components) {
										Update(components..., archetype->Entities.size(), deltaTime);
									},
									std::make_tuple(reinterpret_cast<T*>(archetype->GetComponents<T>().data())...));
						}
					}

					virtual void Update(T*..., uint64_t numEntities, float deltaTime) = 0;

				private:
					DynamicBitSet Signature {};
			};

			class ArchetypeBase
			{
					friend ECS;

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

				// Copies each component into the archetype vector for it
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
			{
				static_assert(std::is_base_of<SystemBase, T>::value, "an ECS System needs to derive from SplitEngine::ECS::System");

				_systems.emplace_back(new T());
			}

			[[nodiscard]] std::vector<ArchetypeBase*> GetArchetypesWithSignature(const DynamicBitSet& signature) const
			{
				std::vector<ArchetypeBase*> archetypes {};

				for (ArchetypeBase* archetype : ArchetypeBase::_archetypes)
				{
					if (signature.FuzzyMatches(archetype->Signature)) { archetypes.push_back(archetype); }
				}

				return archetypes;
			}

			template<typename... T>
			Archetype<T...>& GetArchetype()
			{
				static Archetype<T...> archetype;
				return archetype;
			}

		private:
			uint64_t _entityID    = 0;

			std::unordered_map<uint64_t, size_t> _entityLocation;

			std::vector<SystemBase*> _systems;
	};

	inline std::vector<ECS::ArchetypeBase*> ECS::ArchetypeBase::_archetypes = std::vector<ECS::ArchetypeBase*>();
	inline uint64_t                         ECS::ArchetypeBase::_id         = 0;
}
