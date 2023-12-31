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
				std::vector<ECS::ArchetypeBase*> archetypes = context.Registry->GetArchetypesWithSignature(Signature);

				ExecuteArchetypes(archetypes, context);
			}

			virtual void ExecuteArchetypes(std::vector<ECS::ArchetypeBase*>& archetypes, Context& context)
			{
				for (ECS::ArchetypeBase* archetype : archetypes)
				{
					std::apply(
							[this, &archetype, &context](T*... components) {
								Execute(components..., archetype->Entities, context);
							},
							std::make_tuple(reinterpret_cast<T*>(archetype->GetComponentsRaw<T>().data())...));
				}
			}

			virtual void Execute(T*..., std::vector<uint64_t>& entities, Context& context) {};

		private:
			DynamicBitSet Signature {};
	};
}
