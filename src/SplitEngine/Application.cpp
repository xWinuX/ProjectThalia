#define GLM_FORCE_DEPTH_ZERO_TO_ONE




#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Input.hpp"
#include "SplitEngine/Debug/Log.hpp"

#include <format>

#include <chrono>
#include <utility>

#ifndef SE_HEADLESS
	#include <imgui.h>
#endif

namespace SplitEngine
{
	ApplicationInfo Application::_applicationInfo = {};

	Application::Application(ApplicationInfo applicationInfo) { _applicationInfo = std::move(applicationInfo); }

	void Application::Initialize()
	{
		if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
		{
			ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError()));
		}

#ifndef SE_HEADLESS
		_renderer.Initialize();
#endif
	}

	void Application::Run()
	{
		Initialize();

		// Main Loop
		uint64_t currentTime = SDL_GetPerformanceCounter();
		uint64_t previousTime;
		float    deltaTime;

		uint64_t averageFps           = 0;
		float    averageDeltaTime     = 0.0f;
		float    accumulatedDeltaTime = 0.0f;
		uint64_t accumulatedFrames    = 0;

		SDL_Event event;
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


			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_KEYDOWN:
					case SDL_KEYUP: Input::Update(event); break;
					case SDL_QUIT: quit = true; break;
				}
#ifndef SE_HEADLESS
				_renderer.HandleEvents(event);
#endif
			}

			// TODO: Run ECS here
			LOG("Update");


#ifndef SE_HEADLESS
			ImGui::Text("DT: %f", averageDeltaTime);
			ImGui::Text("Frame Time (ms): %f", averageDeltaTime / 0.001f);
			ImGui::Text("FPS: %llu", averageFps);

			_renderer.Render();
#endif
			Input::Reset();
		}
	}

	const ApplicationInfo& Application::GetApplicationInfo() { return _applicationInfo; }
}
