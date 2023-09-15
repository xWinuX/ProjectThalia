#include "ProjectThalia/Application.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Window.hpp"

#include <format>

void ProjectThalia::Application::Run()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) { ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {}\n", SDL_GetError())); }

	_window.Open();

	// Main Loop
	SDL_Event e;

	bool quit = false;
	while (!quit)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) { quit = true; }
		}
	}

	_window.Close();
}
