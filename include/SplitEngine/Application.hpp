#pragma once
#define SDL_MAIN_HANDLED

// TODO: Investigate why this is so broken
// This needs to be here for it to work for some reason
#ifndef SE_HEADLESS
	#include "SplitEngine/Rendering/Renderer.hpp"
#endif

#include "ApplicationInfo.hpp"
#include "Event.hpp"
#include "Window.hpp"

#include <array>

namespace SplitEngine
{
	class Application
	{
		public:
			explicit Application(ApplicationInfo applicationInfo);
			void Run();

			static const ApplicationInfo& GetApplicationInfo();

		private:
			static ApplicationInfo _applicationInfo;

			void Initialize();

#ifndef SE_HEADLESS
			Rendering::Renderer _renderer;
#endif
	};
}