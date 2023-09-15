#include "ProjectThalia/Window.hpp"
#include "ProjectThalia/ErrorHandler.hpp"

#include <format>


namespace ProjectThalia
{
	void Window::Open()
	{
		const int SCREEN_WIDTH  = 500;
		const int SCREEN_HEIGHT = 500;

		// Create _window
		_window = SDL_CreateWindow("Project Thalia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
		if (_window == nullptr) { ErrorHandler::ThrowRuntimeError(std::format("Window could not be created! SDL_Error: {}\n", SDL_GetError())); }
		else { _vulkanContext.Initialize(_window); }
	}

	void Window::Close()
	{
		_vulkanContext.Destroy();

		SDL_DestroyWindow(_window);
		SDL_Quit();
	}
}