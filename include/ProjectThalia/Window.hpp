#pragma once

#include "Rendering/VulkanContext.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace ProjectThalia
{
	class Window
	{
		public:
			void Open();
			void Close();

			SDL_Window* GetSDLWindow();

		private:
			SDL_Window* _window = nullptr;
	};
}
