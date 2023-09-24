#include "ProjectThalia/Application.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"

#include <format>

namespace ProjectThalia
{
	void Application::Run()
	{
		Initialize();

		// Main Loop
		SDL_Event e;
		Debug::Log::Info("BEFORE LOOP");
		bool quit = false;
		while (!quit)
		{
			_renderer.DrawFrame();

			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT) { quit = true; }
			}
		}

		Destroy();
	}

	void Application::Destroy()
	{
		this->_renderer.Destroy();
		this->_window.Close();
	}

	void Application::Initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
		{
			ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError()));
		}

		_window.Open();
		_renderer.Initialize(_window);
	}
}
