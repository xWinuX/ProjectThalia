#include "SplitEngine/Window.hpp"
#include "SplitEngine/ErrorHandler.hpp"

#include <format>

#include "SplitEngine/Debug/Log.hpp"

namespace SplitEngine
{
	Window::Window(const std::string& windowTitle, uint32_t width, uint32_t height)
	{
		LOG("Initializing Window...");
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) { ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError())); }

		LOG("Opening Window...");
		_window = SDL_CreateWindow(windowTitle.c_str(),
		                           SDL_WINDOWPOS_CENTERED,
		                           SDL_WINDOWPOS_CENTERED,
		                           static_cast<int>(width),
		                           static_cast<int>(height),
		                           SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

		if (_window == nullptr) { ErrorHandler::ThrowRuntimeError(std::format("Window could not be created! SDL_Error: {0}\n", SDL_GetError())); }
	}

	void Window::Close()
	{
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}

	SDL_Window* Window::GetSDLWindow() const { return _window; }

	glm::ivec2 Window::GetSize() const
	{
		glm::ivec2 size;

		SDL_GetWindowSize(_window, &size.x, &size.y);

		return size;
	}

	bool Window::IsMinimized() const { return _isMinimized; }

	void Window::HandleEvents(SDL_Event event)
	{
		switch (event.window.event)
		{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				OnResize.Invoke(event.window.data1, event.window.data2);
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				SetMinimized(true);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				SetMinimized(false);
				break;
		}
	}

	void Window::SetSize(uint32_t width, uint32_t height)
	{
		SDL_SetWindowSize(_window, static_cast<int>(width), static_cast<int>(height));
	}

	void Window::SetFullscreen(bool fullscreen)
	{
		SDL_SetWindowFullscreen(_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
		_isInFullscreen = fullscreen;
	}

	void Window::ToggleFullscreen()
	{
		SetFullscreen(!_isInFullscreen);
	}

	void Window::SetMinimized(bool newState) { _isMinimized = newState; }
}
