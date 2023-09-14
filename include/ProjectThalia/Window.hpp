#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "vulkan/vulkan.h"

namespace ProjectThalia
{
	class Window
	{
		public:
			void Open();

		private:
			SDL_Window*  _window;
			VkInstance   _vulkanInstance;
			VkSurfaceKHR _vulkanSurface;
	};
}
