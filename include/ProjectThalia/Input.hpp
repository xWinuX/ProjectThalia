#pragma once

#include <SDL_events.h>
#include <unordered_map>

namespace ProjectThalia
{
	class Application;

	class Input
	{
			friend Application;

		public:
			static bool GetDown(SDL_KeyCode keyCode);
			static bool GetPressed(SDL_KeyCode keyCode);

		private:
			enum PressedState
			{
				Ready,
				Pressed,
				Waiting,
			};

			static std::unordered_map<int, bool> _keyDownStates;
			static std::unordered_map<int, PressedState> _keyPressedStates;

			static void Update(const SDL_Event& event);
			static void Reset();
	};

}