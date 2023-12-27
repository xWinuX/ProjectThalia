#pragma once

namespace SplitEngine::ECS
{
	class Registry;

	class SystemBase
	{
		public:
			virtual ~SystemBase() {};
			virtual void RunExecute(Context& context) = 0;
	};
}