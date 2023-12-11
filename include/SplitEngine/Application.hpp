#pragma once
#define SDL_MAIN_HANDLED

#include "ApplicationInfo.hpp"
#include "Event.hpp"
#include "Window.hpp"

//#ifndef SE_HEADLESS
	#include "SplitEngine/Rendering/Renderer.hpp"
//#endif

#include <array>

namespace SplitEngine
{
	class Application
	{
		public:
			explicit Application(ApplicationInfo applicationInfo);
			~Application();
			void Run();

			static const ApplicationInfo& GetApplicationInfo();

		private:
			static ApplicationInfo _applicationInfo;

			void Initialize();

//#ifndef SE_HEADLESS
			Rendering::Renderer _renderer;
//#endif
	};
}