#pragma once

#ifndef SE_HEADLESS
namespace SplitEngine
{
	namespace Rendering::Vulkan
	{
		class Context;
	}

	namespace Audio
	{
		class Manager;
	}

	class AssetDatabase;
}
#endif

namespace SplitEngine::ECS
{
	class Registry;

	struct Context
	{
			Registry* Registry;
			float     DeltaTime;
			SplitEngine::AssetDatabase* AssetDatabase;
#ifndef SE_HEADLESS
			SplitEngine::Rendering::Vulkan::Context* RenderingContext;
			SplitEngine::Audio::Manager* AudioManager;
#endif
	};
}
