#pragma once

#include "Archetype.hpp"
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

			void RunExecute(ContextProvider& context, uint8_t stage) final
			{
				if (!_cachedArchetypes)
				{
					_archetypes = context.Registry->GetArchetypesWithSignature(Signature);
					_cachedArchetypes = true;
				}

				ExecuteArchetypes(_archetypes, context);
			}

			virtual void ExecuteArchetypes(std::vector<Archetype*>& archetypes, ContextProvider& context)
			{
				for (Archetype* archetype: archetypes)
				{
					std::apply([this, &archetype, &context](T*... components) { Execute(components..., archetype->Entities, context); },
					           std::make_tuple(reinterpret_cast<T*>(archetype->GetComponentsRaw<T>().data())...));
				}
			}

			virtual void Execute(T*..., std::vector<uint64_t>& entities, ContextProvider& context) {}

		private:
			std::vector<Archetype*> _archetypes;

			DynamicBitSet Signature{};
	};
}
