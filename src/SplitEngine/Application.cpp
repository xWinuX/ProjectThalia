#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "SplitEngine/Application.hpp"
#include "SplitEngine/Debug/Log.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Input.hpp"

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
		LOG("Initializing SDL Events...");

		if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
		{
			ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError()));
		}

#ifndef SE_HEADLESS
		LOG("Initializing Renderer...");
		_renderer.Initialize();

		LOG("Initializing Audio...");
		_audioManager.Initialize();

		LOG("Initializing ECS...");
		_ecsRegistry.RegisterRenderingContext(&_renderer.GetContext());
		_ecsRegistry.RegisterAudioManager(&_audioManager);

#endif
		_ecsRegistry.RegisterAssetDatabase(&_assetDatabase);
	}

	void Application::Run()
	{
		// Main Loop
		uint64_t currentTime  = SDL_GetPerformanceCounter();
		uint64_t previousTime = 0;
		float    deltaTime    = 0;

		uint64_t averageFps       = 0;
		float    averageDeltaTime = 0.0f;

		float    accumulatedDeltaTime = 0.0f;
		uint64_t accumulatedFrames    = 0;


		float accumulatedUpdateTime = 0.0f;
		float averageUpdateTime     = 0.0f;

		uint64_t updateStartTime = 0;
		uint64_t updateEndTime   = 0;

		float accumulatedRenderTime = 0.0f;
		float averageRenderTime     = 0.0f;

		uint64_t renderStartTime = 0;
		uint64_t renderEndTime   = 0;

		LOG("Start Game Loop");
		SDL_Event event;
		bool      quit = false;
		while (!quit)
		{
			previousTime = currentTime;
			currentTime  = SDL_GetPerformanceCounter();

			deltaTime = (static_cast<float>((currentTime - previousTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;

			accumulatedUpdateTime += static_cast<float>((updateEndTime - updateStartTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency());
			accumulatedRenderTime += static_cast<float>((renderEndTime - renderStartTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency());

			accumulatedDeltaTime += deltaTime;

			accumulatedFrames++;

			if (accumulatedDeltaTime >= 0.5f)
			{
				averageDeltaTime  = accumulatedDeltaTime / static_cast<float>(accumulatedFrames);
				averageUpdateTime = accumulatedUpdateTime / static_cast<float>(accumulatedFrames);
				averageRenderTime = accumulatedRenderTime / static_cast<float>(accumulatedFrames);

				averageFps = static_cast<uint64_t>((1.0f / averageDeltaTime));

				accumulatedDeltaTime  = 0.0f;
				accumulatedUpdateTime = 0.0f;
				accumulatedRenderTime = 0.0f;

				accumulatedFrames = 0;
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

			updateStartTime = SDL_GetPerformanceCounter();
			_ecsRegistry.Update(deltaTime);
			updateEndTime = SDL_GetPerformanceCounter();

#ifndef SE_HEADLESS
			ImGui::Text("DT: %f", averageDeltaTime);
			ImGui::Text("Frame Time (ms): %f", averageDeltaTime / 0.001f);
			ImGui::Text("ECS Time (ms): %f", averageUpdateTime);
			ImGui::Text("Render Time (ms): %f", averageRenderTime);
			ImGui::Text("FPS: %llu", averageFps);

			renderStartTime = SDL_GetPerformanceCounter();
			_renderer.BeginRender();

			_ecsRegistry.Render(deltaTime);

			_renderer.EndRender();
			renderEndTime = SDL_GetPerformanceCounter();
#endif
			Input::Reset();
		}

#ifndef SE_HEADLESS
		LOG("Waiting for frame to finish...");
		_renderer.GetContext().WaitForIdle();
#endif
	}

	const ApplicationInfo& Application::GetApplicationInfo() { return _applicationInfo; }

	AssetDatabase& Application::GetAssetDatabase() { return _assetDatabase; }

	ECS::Registry& Application::GetECSRegistry() { return _ecsRegistry; }

}
