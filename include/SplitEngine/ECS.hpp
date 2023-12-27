#pragma once

#include "SplitEngine/DataStructures.hpp"
#include "SplitEngine/Debug/Log.hpp"
#include <bitset>
#include <chrono>
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

			struct Entity
			{
				public:
					uint64_t archetypeIndex;
					uint64_t componentIndex;
			};

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
										Update(components..., archetype->Entities, deltaTime);
									},
									std::make_tuple(reinterpret_cast<T*>(archetype->GetComponents<T>().data())...));
						}
					}

					virtual void Update(T*..., std::vector<uint64_t>& entities, float deltaTime) = 0;

				private:
					DynamicBitSet Signature {};
			};

			class ArchetypeBase
			{
					friend ECS;

				public:
					uint64_t                            ID = 0;
					std::vector<uint64_t>               Entities {};
					std::vector<std::vector<std::byte>> ComponentData {};
					DynamicBitSet                       Signature {};

					template<typename T>
					std::vector<std::byte>& GetComponents()
					{
						return ComponentData[TypeIDGenerator<Component>::GetID<T>()];
					}

					template<typename ...T>
					uint64_t AddEntity(uint64_t entityID, T&&... args)
					{
						Entities.push_back(entityID);

						((std::copy(reinterpret_cast<const std::byte*>(&args),
									reinterpret_cast<const std::byte*>(&args) + sizeof(args),
									std::back_inserter(GetComponents<T>()))),
						 ...);

						return Entities.size() - 1;
					}

					void DestroyEntity(uint64_t entityID) { RunDestroyEntity(entityID); }

				protected:
					static uint64_t                    _id;
					static std::vector<ArchetypeBase*> _archetypes;

					virtual void RunDestroyEntity(uint64_t entityID) = 0;
			};

			template<typename... T>
			class Archetype : public ArchetypeBase
			{
				public:
					explicit Archetype(std::vector<Entity>& sparseEntityLookup) :
						_sparseEntityLookup(sparseEntityLookup)
					{
						Signature.ExtendSizeBy(TypeIDGenerator<Component>::GetCount());
						ComponentData.resize(TypeIDGenerator<Component>::GetCount());
						(
								[&] {
									uint64_t id = TypeIDGenerator<Component>::GetID<T>();
									Signature.SetBit(id);
								}(),
								...);

						ID = _id++;

						_archetypes.push_back(this);
					}

					void RunDestroyEntity(uint64_t entityID) override
					{
						size_t indexToRemove = _sparseEntityLookup[entityID].componentIndex;
						size_t lastIndex     = Entities.size() - 1;

						_sparseEntityLookup[Entities[lastIndex]].componentIndex = indexToRemove;

						std::swap(Entities[indexToRemove], Entities[lastIndex]);
						Entities.pop_back();

						(
								[&] {
									std::vector<std::byte>& bytes = GetComponents<T>();
									T*                      data  = reinterpret_cast<T*>(bytes.data());
									std::swap(data[indexToRemove], data[lastIndex]);
									bytes.erase(bytes.end() - sizeof(T), bytes.end());
								}(),
								...);
					}

				private:
					std::vector<Entity>& _sparseEntityLookup;
			};

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

			void DestroyEntity(uint64_t entityID)
			{
				ArchetypeBase::_archetypes[_sparseEntityLookup[entityID].archetypeIndex]->DestroyEntity(entityID);

				_entityGraveyard.Push(entityID);
			}

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

			[[nodiscard]] static std::vector<ArchetypeBase*> GetArchetypesWithSignature(const DynamicBitSet& signature)
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
				static Archetype<T...> archetype = Archetype<T...>(_sparseEntityLookup);
				return archetype;
			}


		private:
			std::vector<Entity> _sparseEntityLookup;

			AvailableStack<uint64_t> _entityGraveyard;

			std::vector<SystemBase*> _systems;
	};

	inline std::vector<ECS::ArchetypeBase*> ECS::ArchetypeBase::_archetypes = std::vector<ECS::ArchetypeBase*>();
	inline uint64_t                         ECS::ArchetypeBase::_id         = 0;
}
