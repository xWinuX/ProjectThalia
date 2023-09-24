#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/vec2.hpp>

namespace ProjectThalia
{
	class Window
	{
		public:
			Window() = default;

			void Open();
			void Close();

			SDL_Window* GetSDLWindow();

			glm::ivec2 GetSize();

		private:
			SDL_Window* _window = nullptr;
	};
}
