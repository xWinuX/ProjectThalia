#include "SplitEngine/Systems.hpp"

#include "SplitEngine/Contexts.hpp"
#include "SplitEngine/Input.hpp"

#include "SplitEngine/Application.hpp"
#include "SplitEngine/Stages.hpp"

#include <SDL2/SDL_timer.h>

namespace SplitEngine
{
	void StatisticsSystem::RunExecute(ECS::ContextProvider& contextProvider, uint8_t stage)
	{
		Statistics& statistics = contextProvider.GetContext<EngineContext>()->Statistics;

		float deltaTime = contextProvider.GetContext<TimeContext>()->DeltaTime;

		_accumulatedDeltaTime += deltaTime;

		_accumulatedFrames++;

		if (_accumulatedDeltaTime >= 0.5f)
		{
			float averageDeltaTime = _accumulatedDeltaTime / static_cast<float>(_accumulatedFrames);

			statistics.AverageFPS       = static_cast<uint64_t>((1.0f / averageDeltaTime));
			statistics.AverageDeltaTime = averageDeltaTime;

			_accumulatedDeltaTime = 0.0f;

			std::vector<uint8_t>& activeStages           = contextProvider.Registry->GetActiveStages();
			std::vector<float>&   accumulatedStageTimeMs = contextProvider.Registry->GetAccumulatedStageTimeMs();

			for (const uint8_t activeStage: activeStages)
			{
				statistics.AverageECSStageTimeMs[activeStage] = accumulatedStageTimeMs[activeStage] / static_cast<float>(_accumulatedFrames);

				accumulatedStageTimeMs[activeStage] = 0;
			}

			_accumulatedFrames = 0;
		}
	}

	TimeSystem::TimeSystem() { _currentTime = SDL_GetPerformanceCounter(); }

	void TimeSystem::RunExecute(ECS::ContextProvider& contextProvider, uint8_t stage)
	{
		_previousTime = _currentTime;
		_currentTime  = SDL_GetPerformanceCounter();

		float deltaTime = (static_cast<float>((_currentTime - _previousTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;

		contextProvider.GetContext<TimeContext>()->DeltaTime = deltaTime;
	}

	void SDLEventSystem::RunExecute(ECS::ContextProvider& contextProvider, uint8_t stage)
	{
		Input::Reset();
		while (SDL_PollEvent(&_event))
		{
			switch (_event.type)
			{
				case SDL_QUIT:
					contextProvider.GetContext<EngineContext>()->Application->Quit();
					break;
			}

			Input::Update(_event);
			contextProvider.GetContext<RenderingContext>()->Renderer->HandleEvents(_event);
			OnPollEvent.Invoke(_event);
		}
	}

	void RenderingSystem::RunExecute(ECS::ContextProvider& contextProvider, uint8_t stage)
	{
		if (stage == EngineStage::BeginRendering) { contextProvider.GetContext<RenderingContext>()->Renderer->BeginRender(); }

		if (stage == EngineStage::EndRendering) { contextProvider.GetContext<RenderingContext>()->Renderer->EndRender(); }
	}
}
