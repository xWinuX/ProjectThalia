#pragma once

#include "VulkanContext.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace ProjectThalia
{
	class Window
	{
		public:
			void Open();
			void Close();

		private:
			SDL_Window*     _window = nullptr;
			Vulkan::VulkanContext _vulkanContext;
	};
}
