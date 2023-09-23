#pragma once

#include "glm/vec2.hpp"
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

			glm::ivec2 GetSize();

		private:
			SDL_Window* _window = nullptr;

	};
}
