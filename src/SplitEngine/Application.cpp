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


#define PRIVATE_TIME_MEASURE_INIT(varName) \
	uint64_t varName##StartTime = 0;       \
	uint64_t varName##EndTime   = 0;       \
                                           \
	float varName##AccumulatedTime = 0.0f; \
	float varName##AverageTime     = 0.0f;

#define PRIVATE_TIME_MEASURE_ACCUMULATE(varName) \
	varName##AccumulatedTime += static_cast<float>((varName##EndTime - varName##StartTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency());

#define PRIVATE_TIME_MEASURE_AVERAGE(varName)                                                    \
	varName##AverageTime     = varName##AccumulatedTime / static_cast<float>(accumulatedFrames); \
	varName##AccumulatedTime = 0.0f;

#define PRIVATE_TIME_MEASURE_BEGIN(varName) varName##StartTime = SDL_GetPerformanceCounter();

#define PRIVATE_TIME_MEASURE_END(varName) varName##EndTime = SDL_GetPerformanceCounter();

namespace SplitEngine
{
#ifndef SE_HEADLESS
	Application::Application(CreateInfo createInfo):
		_applicationInfo(std::move(createInfo.ApplicationInfo)),
		_renderer(Rendering::Renderer(_applicationInfo, std::move(createInfo.RenderingSettings))),
		_audioManager(Audio::Manager())
#else
	Application::Application(CreateInfo createInfo):
		_info(std::move(createInfo.Info))
#endif
	{
		LOG("Initializing SDL Events...");
		if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) { ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError())); }
#ifndef SE_HEADLESS


		LOG("Initializing ECS...");
		_ecsRegistry.RegisterRenderer(&_renderer);
		_ecsRegistry.RegisterAudioManager(&_audioManager);

#endif
		_ecsRegistry.RegisterApplication(this);
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

		PRIVATE_TIME_MEASURE_INIT(ecsPrepare)
		PRIVATE_TIME_MEASURE_INIT(ecsGameplaySystem)
		PRIVATE_TIME_MEASURE_INIT(ecsRenderSystem)
		PRIVATE_TIME_MEASURE_INIT(renderBegin)
		PRIVATE_TIME_MEASURE_INIT(renderEnd)

		LOG("Start Game Loop");
		SDL_Event event;
		bool      quit = false;
		while (!quit)
		{
			previousTime = currentTime;
			currentTime  = SDL_GetPerformanceCounter();

			deltaTime = (static_cast<float>((currentTime - previousTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;

			PRIVATE_TIME_MEASURE_ACCUMULATE(ecsPrepare)
			PRIVATE_TIME_MEASURE_ACCUMULATE(ecsGameplaySystem)
			PRIVATE_TIME_MEASURE_ACCUMULATE(ecsRenderSystem)
			PRIVATE_TIME_MEASURE_ACCUMULATE(renderBegin)
			PRIVATE_TIME_MEASURE_ACCUMULATE(renderEnd)

			accumulatedDeltaTime += deltaTime;

			accumulatedFrames++;

			if (accumulatedDeltaTime >= 0.5f)
			{
				averageDeltaTime = accumulatedDeltaTime / static_cast<float>(accumulatedFrames);

				PRIVATE_TIME_MEASURE_AVERAGE(ecsPrepare)
				PRIVATE_TIME_MEASURE_AVERAGE(ecsGameplaySystem)
				PRIVATE_TIME_MEASURE_AVERAGE(ecsRenderSystem)
				PRIVATE_TIME_MEASURE_AVERAGE(renderBegin)
				PRIVATE_TIME_MEASURE_AVERAGE(renderEnd)

				_statistics.AverageFPS                = static_cast<uint64_t>((1.0f / averageDeltaTime));
				_statistics.AverageDeltaTime          = averageDeltaTime;
				_statistics.AverageECSPrepareTime     = ecsPrepareAverageTime;
				_statistics.AverageGameplaySystemTime = ecsGameplaySystemAverageTime;
				_statistics.AverageRenderSystemTime   = ecsRenderSystemAverageTime;
				_statistics.AverageRenderBeginTime    = renderBeginAverageTime;
				_statistics.AverageRenderEndTime      = renderEndAverageTime;

				accumulatedDeltaTime = 0.0f;

				accumulatedFrames = 0;
			}

			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_KEYDOWN:
					case SDL_KEYUP:
					case SDL_MOUSEMOTION:
					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP:
						Input::Update(event);
						break;
					case SDL_QUIT:
						quit = true;
						break;
				}
#ifndef SE_HEADLESS
				_renderer.HandleEvents(event);
#endif
			}

			PRIVATE_TIME_MEASURE_BEGIN(ecsPrepare)
			_ecsRegistry.PrepareForExecution(deltaTime);
			PRIVATE_TIME_MEASURE_END(ecsPrepare)

			PRIVATE_TIME_MEASURE_BEGIN(ecsGameplaySystem)
			_ecsRegistry.ExecuteSystems(ECS::Stage::Gameplay);
			PRIVATE_TIME_MEASURE_END(ecsGameplaySystem)

#ifndef SE_HEADLESS

			PRIVATE_TIME_MEASURE_BEGIN(renderBegin)
			_renderer.BeginRender();
			PRIVATE_TIME_MEASURE_END(renderBegin)

			PRIVATE_TIME_MEASURE_BEGIN(ecsRenderSystem)
			if (!_renderer.WasSkipped()) { _ecsRegistry.ExecuteSystems(ECS::Stage::Rendering); }
			PRIVATE_TIME_MEASURE_END(ecsRenderSystem)

			PRIVATE_TIME_MEASURE_BEGIN(renderEnd)
			_renderer.EndRender();
			PRIVATE_TIME_MEASURE_END(renderEnd)

#endif
			Input::Reset();
		}

#ifndef SE_HEADLESS
		LOG("Waiting for frame to finish...");
		_renderer.GetVulkanInstance().GetPhysicalDevice().GetDevice().WaitForIdle();
#endif
	}

	Window& Application::GetWindow() { return _renderer.GetWindow(); }

	Application::Statistics& Application::GetStatistics() { return _statistics; }

	AssetDatabase& Application::GetAssetDatabase() { return _assetDatabase; }

	ECS::Registry& Application::GetECSRegistry() { return _ecsRegistry; }
}
