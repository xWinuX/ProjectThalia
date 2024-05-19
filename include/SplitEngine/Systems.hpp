#pragma once
#include <cstdint>
#include <SDL_events.h>

#include "ECS/ContextProvider.hpp"
#include "ECS/SystemBase.hpp"

namespace SplitEngine
{
	class StatisticsSystem : public ECS::SystemBase
	{
		public:
			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;

		private:
			float    _accumulatedDeltaTime = 0.0f;
			uint64_t _accumulatedFrames    = 0;
	};

	class TimeSystem : public ECS::SystemBase
	{
		public:
			TimeSystem();

			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;

		private:
			uint64_t _currentTime  = 0;
			uint64_t _previousTime = 0;
	};

	class SDLEventSystem : public ECS::SystemBase
	{
		public:
			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;

		private:
			SDL_Event _event{};
	};


	class RenderingSystem : public ECS::SystemBase
	{
		public:
			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;
	};
}
