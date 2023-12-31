#pragma once

#include "Registry.hpp"
#include "SystemBase.hpp"

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

			void RunExecute(Context& context) final
			{
				std::vector<ArchetypeBase*> archetypes = context.Registry->GetArchetypesWithSignature(Signature);

				Execute(archetypes, context);
			}

			virtual void Execute(std::vector<ArchetypeBase*>& archetypes, Context& context)
			{
				for (ArchetypeBase* archetype : archetypes)
				{
					std::apply(
							[this, &archetype, &context](T*... components) {
								Execute(components..., archetype->Entities, context);
							},
							std::make_tuple(reinterpret_cast<T*>(archetype->GetComponents<T>().data())...));
				}
			}

			virtual void Execute(T*..., std::vector<uint64_t>& entities, Context& context) {};

		private:
			DynamicBitSet Signature {};
	};
}
