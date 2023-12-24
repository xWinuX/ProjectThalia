#pragma once
#define SDL_MAIN_HANDLED

// TODO: Investigate why this is so broken
// This needs to be here for it to work for some reason
#ifndef SE_HEADLESS
	#include "SplitEngine/Rendering/Renderer.hpp"
#endif

#include "ApplicationInfo.hpp"
#include "AssetDatabase.hpp"
#include "ECS.hpp"
#include "Event.hpp"
#include "Window.hpp"

#include <array>

namespace SplitEngine
{
	class Application
	{
		public:
			explicit Application(ApplicationInfo applicationInfo);
			void Initialize();
			void Run();

			static const ApplicationInfo& GetApplicationInfo();

			AssetDatabase& GetAssetDatabase();
			ECS&           GetECS();

		private:
			static ApplicationInfo _applicationInfo;

#ifndef SE_HEADLESS
			Rendering::Renderer _renderer;
#endif

			AssetDatabase _assetDatabase;
			ECS           _ecs;
	};
}