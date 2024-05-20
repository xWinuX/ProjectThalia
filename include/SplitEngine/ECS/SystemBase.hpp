#pragma once

namespace SplitEngine::ECS
{
	class Registry;

	class SystemBase
	{
		friend class Registry;

		public:
			virtual ~SystemBase() = default;

		protected:
			virtual void Destroy(ContextProvider& contextProvider) {}
			virtual void RunExecute(ContextProvider& context, uint8_t stage) = 0;

			bool _cachedArchetypes = false;
	};
}
