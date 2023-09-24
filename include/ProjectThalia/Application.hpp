#pragma once
#define SDL_MAIN_HANDLED

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

			Window              _window;
			Rendering::Renderer _renderer;
	};
}