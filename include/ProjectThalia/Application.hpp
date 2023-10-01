#pragma once
#define SDL_MAIN_HANDLED

#include "Event.hpp"
#include "ProjectThalia/Rendering/Renderer.hpp"
#include "Window.hpp"

namespace ProjectThalia
{
	class Application
	{
		public:
			Application() = default;
			void Run();


		private:
			void Initialize();
			void Destroy();

			Event<int>          _event;
			Window              _window;
			Rendering::Renderer _renderer;
	};
}