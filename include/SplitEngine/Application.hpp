#pragma once
#define SDL_MAIN_HANDLED

// TODO: Investigate why this is so broken
// This needs to be here for it to work for some reason
#ifndef SE_HEADLESS
#include "SplitEngine/Rendering/Renderer.hpp"
#endif

#include "ApplicationInfo.hpp"
#include "RenderingSettings.hpp"
#include "AssetDatabase.hpp"
#include "ECS/Registry.hpp"
#include "SplitEngine/Audio/Manager.hpp"

namespace SplitEngine
{
	class Application
	{
		public:
			struct Statistics
			{
				uint64_t AverageFPS;
				float    AverageGameplaySystemTime;
				float    AverageRenderSystemTime;
				float    AverageRenderBeginTime;
				float    AverageRenderEndTime;
			};

			struct CreateInfo
			{
				ApplicationInfo   ApplicationInfo{};
				RenderingSettings RenderingSettings{};
			};

			explicit Application(CreateInfo createInfo);
			void     Run();

			[[nodiscard]] Window& GetWindow();
			[[nodiscard]] Statistics& GetStatistics();

			AssetDatabase& GetAssetDatabase();
			ECS::Registry& GetECSRegistry();

		private:
			ApplicationInfo _applicationInfo{};

			Statistics _statistics{};

#ifndef SE_HEADLESS
			Rendering::Renderer _renderer;
			Audio::Manager      _audioManager;
#endif
			ECS::Registry _ecsRegistry;

			AssetDatabase _assetDatabase;
	};
}
