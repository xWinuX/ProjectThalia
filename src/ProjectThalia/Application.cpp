#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "ProjectThalia/Application.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

#include <format>

namespace ProjectThalia
{
	void Application::Run()
	{
		Initialize();

		// Main Loop
		uint64_t currentTime = SDL_GetPerformanceCounter();
		uint64_t previousTime;
		float    deltaTime;

		uint64_t averageFps           = 0;
		float    averageDeltaTime     = 0.0f;
		float    accumulatedDeltaTime = 0;
		uint64_t accumulatedFrames    = 0;

		SDL_Event e;
		bool      quit = false;
		while (!quit)
		{
			previousTime = currentTime;
			currentTime  = SDL_GetPerformanceCounter();

			deltaTime = (static_cast<float>((currentTime - previousTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;

			accumulatedDeltaTime += deltaTime;
			accumulatedFrames++;

			if (accumulatedDeltaTime >= 0.1f)
			{
				averageDeltaTime = accumulatedDeltaTime / static_cast<float>(accumulatedFrames);
				averageFps       = static_cast<uint64_t>((1.0f / averageDeltaTime));

				accumulatedDeltaTime = 0.0f;
				accumulatedFrames    = 0;
			}

			while (SDL_PollEvent(&e))
			{
				ImGui_ImplSDL2_ProcessEvent(&e);

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

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplSDL2_NewFrame(_window.GetSDLWindow());

			ImGui::NewFrame();

			ImGui::Text("DT: %f", averageDeltaTime);
			ImGui::Text("Frame Time (ms): %f", averageDeltaTime/0.001f);
			ImGui::Text("FPS: %llu", averageFps);

			ImGui::Render();

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
