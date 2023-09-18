#pragma once
#define SDL_MAIN_HANDLED

#include "Window.hpp"

namespace ProjectThalia
{
	class Application
	{
		public:
			void Run();

		private:
			Window _window;
			Vulkan::VulkanContext _vulkanContext;
	};
}