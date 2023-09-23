#pragma once
#define SDL_MAIN_HANDLED

#include "ProjectThalia/Rendering/VulkanContext.hpp"
#include "Window.hpp"

namespace ProjectThalia
{
	class Application
	{
		public:
			void Run();

		private:
			Window                   _window;
			Rendering::VulkanContext _vulkanContext;
	};
}