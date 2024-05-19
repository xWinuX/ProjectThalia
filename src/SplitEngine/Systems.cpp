#include "SplitEngine/Systems.hpp"

#include <SDL_timer.h>

#include "SplitEngine/Contexts.hpp"
#include "SplitEngine/Input.hpp"

#include "SplitEngine/Application.hpp"
#include "SplitEngine/Stage.hpp"
#include "SplitEngine/Rendering/Renderer.hpp"


namespace SplitEngine
{
	void StatisticsSystem::RunExecute(ECS::ContextProvider& context, uint8_t stage)
	{
		Statistics& statistics = context.GetContext<EngineContext>()->Statistics;

		float deltaTime = context.GetContext<TimeContext>()->DeltaTime;

		_accumulatedDeltaTime += deltaTime;

		_accumulatedFrames++;

		if (_accumulatedDeltaTime >= 0.5f)
		{
			float averageDeltaTime = _accumulatedDeltaTime / static_cast<float>(_accumulatedFrames);

			statistics.AverageFPS       = static_cast<uint64_t>((1.0f / averageDeltaTime));
			statistics.AverageDeltaTime = averageDeltaTime;

			_accumulatedDeltaTime = 0.0f;

			std::vector<uint8_t>& activeStages           = context.Registry->GetActiveStages();
			std::vector<float>&   accumulatedStageTimeMs = context.Registry->GetAccumulatedStageTimeMs();

			for (const uint8_t activeStage: activeStages)
			{
				statistics.AverageECSStageTimeMs[activeStage] = accumulatedStageTimeMs[activeStage] / static_cast<float>(_accumulatedFrames);

				accumulatedStageTimeMs[activeStage] = 0;
			}

			_accumulatedFrames = 0;
		}
	}

	TimeSystem::TimeSystem() { _currentTime = SDL_GetPerformanceCounter(); }

	void TimeSystem::RunExecute(ECS::ContextProvider& context, uint8_t stage)
	{
		_previousTime = _currentTime;
		_currentTime  = SDL_GetPerformanceCounter();

		float deltaTime = (static_cast<float>((_currentTime - _previousTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;

		context.GetContext<TimeContext>()->DeltaTime = deltaTime;
	}

	void SDLEventSystem::RunExecute(ECS::ContextProvider& context, uint8_t stage)
	{
		Input::Reset();
		while (SDL_PollEvent(&_event))
		{
			switch (_event.type)
			{
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					Input::Update(_event);
					break;
				case SDL_QUIT:
					context.GetContext<EngineContext>()->Application->Quit();
					break;
			}
			context.GetContext<RenderingContext>()->Renderer->HandleEvents(_event);
		}
	}

	void RenderingSystem::RunExecute(ECS::ContextProvider& context, uint8_t stage)
	{
		if (stage == Stage::BeginRendering) { context.GetContext<RenderingContext>()->Renderer->BeginRender(); }

		if (stage == Stage::EndRendering) { context.GetContext<RenderingContext>()->Renderer->EndRender(); }
	}
}
