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
				_signature.ExtendSizeBy(TypeIDGenerator<Component>::GetCount());
				(_signature.SetBit(TypeIDGenerator<Component>::GetID<T>()), ...);
			}

		protected:
			void RunExecute(ContextProvider& contextProvider, uint8_t stage) final
			{
				if (!_cachedArchetypes)
				{
					_archetypes       = contextProvider.Registry->GetArchetypesWithSignature(_signature);
					_cachedArchetypes = true;
				}

				ExecuteArchetypes(_archetypes, contextProvider, stage);
			}

			virtual void ExecuteArchetypes(std::vector<Archetype*>& archetypes, ContextProvider& contextProvider, uint8_t stage)
			{
				for (Archetype* archetype: archetypes)
				{
					std::apply([this, &archetype, &contextProvider, stage](T*... components) { Execute(components..., archetype->Entities, contextProvider, stage); },
					           std::make_tuple(reinterpret_cast<T*>(archetype->GetComponentsRaw<T>().data())...));
				}
			}

			virtual void Execute(T*..., std::vector<uint64_t>& entities, ContextProvider& context, uint8_t stage) {}

		private:
			std::vector<Archetype*> _archetypes;

			DynamicBitSet _signature{};
	};
}
