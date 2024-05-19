#pragma once

namespace SplitEngine::ECS
{
	class Registry;

	class SystemBase
	{
		friend class Registry;

		public:
			virtual      ~SystemBase() = default;
			virtual void RunExecute(ContextProvider& context, uint8_t stage) = 0;

		protected:
			bool _cachedArchetypes = false;
	};
}
