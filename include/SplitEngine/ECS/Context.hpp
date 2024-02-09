#pragma once


namespace SplitEngine
{
#ifndef SE_HEADLESS
	namespace Rendering::Vulkan
	{
		class Instance;
	}

	namespace Audio
	{
		class Manager;
	}
#endif
	class Application;
	class AssetDatabase;
}


namespace SplitEngine::ECS
{
	class Registry;

	struct Context
	{
		float          DeltaTime;
		Application*   Application;
		Registry*      Registry;
		AssetDatabase* AssetDatabase;

#ifndef SE_HEADLESS
		Rendering::Vulkan::Instance* RenderingContext;
		Audio::Manager*              AudioManager;
#endif
	};
}
