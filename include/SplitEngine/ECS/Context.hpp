#pragma once


namespace SplitEngine
{
#ifndef SE_HEADLESS
	namespace Rendering
	{
		class Renderer;

		namespace Vulkan
		{
			class Instance;
		}
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
		Rendering::Renderer* Renderer;
		Audio::Manager*      AudioManager;
#endif
	};
}
