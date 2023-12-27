#pragma once

namespace SplitEngine::ECS
{
	class Registry;

	class SystemBase
	{
		public:
			virtual void RunUpdate(const Registry* registry, float deltaTime) = 0;
	};
}