#pragma once

#include "Event.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/vec2.hpp>

namespace SplitEngine
{

	class Window
	{
		//	friend Rendering::Renderer;

		public:
			Window() = default;

			void Open();
			void Close();

			[[nodiscard]] SDL_Window* GetSDLWindow() const;
			[[nodiscard]] glm::ivec2  GetSize() const;
			[[nodiscard]] bool        IsMinimized() const;

			Event<int, int> OnResize;

		private:
			SDL_Window* _window      = nullptr;
			bool        _isMinimized = false;

			void SetMinimized(bool newState);
	};
}
