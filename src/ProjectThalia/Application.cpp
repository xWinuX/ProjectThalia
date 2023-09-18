#include "ProjectThalia/Application.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Window.hpp"

#include <format>

void ProjectThalia::Application::Run()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) { ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {}\n", SDL_GetError())); }

	_window.Open();
	_vulkanContext.Initialize(_window.GetSDLWindow());

	// Main Loop
	SDL_Event e;

	Debug::Log::Info("BEFORE LOOP");
	bool quit = false;
	while (!quit)
	{
		_vulkanContext.DrawFrame();
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) { quit = true; }
		}
	}

	_vulkanContext.Destroy();
	_window.Close();
}
