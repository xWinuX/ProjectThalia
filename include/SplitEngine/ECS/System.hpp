#pragma once

#include "SystemBase.hpp"
#include "Registry.hpp"

namespace SplitEngine::ECS
{

	template<typename... T>
	class System : public SystemBase
	{
		public:
			System()
			{
				Signature.ExtendSizeBy(TypeIDGenerator<Component>::GetCount());
				(Signature.SetBit(TypeIDGenerator<Component>::GetID<T>()), ...);
			}

			void RunUpdate(const Registry* registry, float deltaTime) final
			{
				std::vector<ArchetypeBase*> archetypes = registry->GetArchetypesWithSignature(Signature);

				for (ArchetypeBase* archetype : archetypes)
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

} // ECS

// SplitEngine
