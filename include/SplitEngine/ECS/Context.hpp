#pragma once

#ifndef SE_HEADLESS
namespace SplitEngine::Rendering::Vulkan
{
	class Context;
}
#endif

namespace SplitEngine::ECS
{
	class Registry;

	struct Context
	{
			Registry* Registry;
			float     DeltaTime;
#ifndef SE_HEADLESS
			SplitEngine::Rendering::Vulkan::Context* RenderingContext;
#endif
	};
}
