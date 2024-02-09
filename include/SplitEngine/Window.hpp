#pragma once

#include <string>

#include "Event.hpp"

#include <glm/vec2.hpp>
#include <SDL2/SDL.h>

namespace SplitEngine
{
	class Window
	{
		public:
			Window(const std::string& windowTitle = "Split Engine Game", uint32_t width = 500, uint32_t height = 500);

			void Close();

			[[nodiscard]] SDL_Window* GetSDLWindow() const;
			[[nodiscard]] glm::ivec2  GetSize() const;
			[[nodiscard]] bool        IsMinimized() const;

			void HandleEvents(SDL_Event event);

			Event<int, int> OnResize{};

		private:
			SDL_Window* _window      = nullptr;
			bool        _isMinimized = false;

			void SetMinimized(bool newState);
	};
}
