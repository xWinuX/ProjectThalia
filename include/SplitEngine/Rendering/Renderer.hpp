#pragma once

#include <SplitEngine/RenderingSettings.hpp>

#include "SplitEngine/ApplicationInfo.hpp"
#include "SplitEngine/Window.hpp"
#include "Vulkan/Instance.hpp"

namespace SplitEngine::Rendering
{
	class Renderer
	{
		public:
			explicit Renderer(ApplicationInfo& applicationInfo, RenderingSettings&& renderingSettings);
			~Renderer();

			void BeginRender();
			void EndRender();

			void HandleEvents(SDL_Event event);

			[[nodiscard]] Vulkan::Instance& GetVulkanInstance();

		private:
			Window           _window;
			Vulkan::Instance _vulkanInstance;

			bool     _wasSkipped             = false;
			uint32_t _latestImageIndexResult = 0;

			bool _frameBufferResized = false;
			void StartImGuiFrame() const;
	};
}
