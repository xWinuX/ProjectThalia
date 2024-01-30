#pragma once
#define SDL_MAIN_HANDLED

// TODO: Investigate why this is so broken
// This needs to be here for it to work for some reason
#ifndef SE_HEADLESS
#include "SplitEngine/Rendering/Renderer.hpp"
#endif

#include "ApplicationInfo.hpp"
#include "AssetDatabase.hpp"
#include "ECS/Registry.hpp"
#include "SplitEngine/Audio/Manager.hpp"

namespace SplitEngine
{
	class Application
	{
		public:
			explicit Application(ApplicationInfo applicationInfo);
			void     Initialize();
			void     Run();

			static const ApplicationInfo& GetApplicationInfo();

			AssetDatabase& GetAssetDatabase();
			ECS::Registry& GetECSRegistry();

		private:
			static ApplicationInfo _applicationInfo;

			#ifndef SE_HEADLESS
			Rendering::Renderer _renderer;
			Audio::Manager      _audioManager;
			#endif
			ECS::Registry _ecsRegistry;

			AssetDatabase _assetDatabase;
	};
}
