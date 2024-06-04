#pragma once
#define SDL_MAIN_HANDLED

// TODO: Investigate why this is so broken
// This needs to be here for it to work for some reason
#include "SplitEngine/Rendering/Renderer.hpp"

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
			struct CreateInfo
			{
				ApplicationInfo      ApplicationInfo{};
				ShaderParserSettings ShaderParserSettings{};
				RenderingSettings    RenderingSettings{};
			};

			explicit Application(CreateInfo createInfo);
			void     Run();
			void     Quit();

			[[nodiscard]] Window& GetWindow();

			AssetDatabase& GetAssetDatabase();
			ECS::Registry& GetECSRegistry();

		private:
			ApplicationInfo _applicationInfo{};

			Rendering::Renderer _renderer;
			Audio::Manager      _audioManager;
			ECS::Registry       _ecsRegistry;

			AssetDatabase _assetDatabase;

			bool _quit = false;
	};
}
