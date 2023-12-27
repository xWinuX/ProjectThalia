#pragma once

#include "ArchetypeBase.hpp"
#include "Entity.hpp"

namespace SplitEngine::ECS
{

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
}
