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

		private:
			static std::unordered_map<int, bool> _keyStates;

			static void Update(const SDL_Event& event);
	};

}