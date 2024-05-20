#pragma once
#include <cstdint>
#include <SDL_events.h>

#include "Event.hpp"
#include "ECS/ContextProvider.hpp"
#include "ECS/SystemBase.hpp"

namespace SplitEngine
{
	class StatisticsSystem : public ECS::SystemBase
	{
		protected:
			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;

		private:
			float    _accumulatedDeltaTime = 0.0f;
			uint64_t _accumulatedFrames    = 0;
	};

	class TimeSystem : public ECS::SystemBase
	{
		public:
			TimeSystem();

		protected:
			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;

		private:
			uint64_t _currentTime  = 0;
			uint64_t _previousTime = 0;
	};

	class SDLEventSystem : public ECS::SystemBase
	{
		public:
			Event<SDL_Event&> OnPollEvent;

		protected:
			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;

		private:
			SDL_Event _event{};
	};


	class RenderingSystem : public ECS::SystemBase
	{
		protected:
			void RunExecute(ECS::ContextProvider& context, uint8_t stage) override;
	};
}
