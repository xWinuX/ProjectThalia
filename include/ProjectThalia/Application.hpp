#pragma once
#define SDL_MAIN_HANDLED

#include "ProjectThalia/Rendering/Vulkan/Context.hpp"
#include "Window.hpp"

namespace ProjectThalia
{
	class Application
	{
		public:
			void Run();

		private:
			Window                     _window;
			Rendering::Vulkan::Context _vulkanContext;
	};
}