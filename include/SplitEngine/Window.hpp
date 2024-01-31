#pragma once

#include "Event.hpp"

#include <glm/vec2.hpp>
#include <SDL2/SDL.h>

namespace SplitEngine
{
	class Window
	{
		public:
			Window() = default;

			void Open();
			void Close();

			[[nodiscard]] SDL_Window* GetSDLWindow() const;
			[[nodiscard]] glm::ivec2  GetSize() const;
			[[nodiscard]] bool        IsMinimized() const;

			Event<int, int> OnResize{};

		private:
			SDL_Window* _window      = nullptr;
			bool        _isMinimized = false;

			void SetMinimized(bool newState);
	};
}
