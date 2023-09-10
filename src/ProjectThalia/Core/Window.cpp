#include "ProjectThalia/Core/Window.hpp"

#include "SDL2/SDL.h"
#include <cstdio>

namespace ProjectThalia::Core
{
	// TODO: write real implementation and open with vulkan context
	// Source: https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
	void Window::Open()
	{
		const int SCREEN_WIDTH  = 500;
		const int SCREEN_HEIGHT = 500;

		//The window we'll be rendering to
		SDL_Window* window = nullptr;

		//The surface contained by the window
		SDL_Surface* screenSurface = nullptr;

		//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			//Create window
			window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
			if (window == nullptr)
			{
				printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			}
			else
			{
				//Get window surface
				screenSurface = SDL_GetWindowSurface(window);

				//Fill the surface white
				SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0x88, 0x88, 0x88));

				//Update the surface
				SDL_UpdateWindowSurface(window);

				//Hack to get window to stay up
				SDL_Event e;
				bool      quit = false;
				while (!quit)
				{
					while (SDL_PollEvent(&e))
					{
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
				}
			}
		}

		//Destroy window
		SDL_DestroyWindow(window);

		//Quit SDL subsystems
		SDL_Quit();
	}
}