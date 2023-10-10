#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
		bool quit = false;
		while (!quit)
		{

			while (SDL_PollEvent(&e))
			{
				switch (e.type)
				{
					case SDL_KEYDOWN: _event.Invoke(0); break;
					case SDL_QUIT: quit = true; break;
				}

				switch (e.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED: _window.OnResize.Invoke(e.window.data1, e.window.data2); break;
					case SDL_WINDOWEVENT_MINIMIZED: _window.SetMinimized(true); break;
					case SDL_WINDOWEVENT_RESTORED: _window.SetMinimized(false); break;
				}
			}


			if (!_window.IsMinimized()) { _renderer.DrawFrame(); }
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
		_renderer.Initialize(&_window);
	}

}
