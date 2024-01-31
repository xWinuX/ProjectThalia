#pragma once

namespace SplitEngine::ECS
{
	class Registry;

	class SystemBase
	{
		public:
			virtual      ~SystemBase() = default;
			virtual void RunExecute(Context& context) = 0;
	};
}
