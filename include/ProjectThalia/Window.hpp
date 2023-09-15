#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "VulkanContext.hpp"
#include "vulkan/vulkan.h"

namespace ProjectThalia
{
	class Window
	{
		public:
			void Open();
			void Close();

		private:
			SDL_Window*  _window;
			VulkanContext _vulkanContext;
	};
}
