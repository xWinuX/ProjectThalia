#include "ProjectThalia/Core/Window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ProjectThalia::Core
{
	void Window::Open()
	{
		const int SCREEN_WIDTH  = 500;
		const int SCREEN_HEIGHT = 500;

		// The window we'll be rendering to
		SDL_Window*  window         = nullptr;
		VkInstance   vulkanInstance = nullptr;
		VkSurfaceKHR vulkanSurface  = nullptr;

		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) { printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError()); }
		else
		{
			// Create window
			window = SDL_CreateWindow("Project Thalia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
			if (window == nullptr) { printf("Window could not be created! SDL_Error: %s\n", SDL_GetError()); }
			else
			{
				// Get vulkan instance extensions
				uint32_t extensionCount;
				SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);

				std::vector<const char*> extensionNames(extensionCount);
				SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames.data());

				// Set vulkan validation layers
				const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

				// Create application info
				VkApplicationInfo applicationInfo = {.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
													 .pApplicationName   = "Project Thalia",
													 .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
													 .pEngineName        = "No Engine",
													 .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
													 .apiVersion         = VK_API_VERSION_1_2};

				// Create instance info
				VkInstanceCreateInfo createInfo = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
												   .pApplicationInfo        = &applicationInfo,
												   .enabledLayerCount       = sizeof(validationLayers) / sizeof(validationLayers[0]),
												   .ppEnabledLayerNames     = validationLayers,
												   .enabledExtensionCount   = extensionCount,
												   .ppEnabledExtensionNames = extensionNames.data()};

				// Create vulkan instance
				VkResult instanceCreationResult = vkCreateInstance(&createInfo, nullptr, &vulkanInstance);
				if (instanceCreationResult != VK_SUCCESS) { throw std::runtime_error("failed to create Vulkan instance!"); }

				// Create vulkan surface
				SDL_bool surfaceCreationResult = SDL_Vulkan_CreateSurface(window, vulkanInstance, &vulkanSurface);
				if (surfaceCreationResult == SDL_FALSE) { throw std::runtime_error("failed to create SDL Vulkan vulkanSurface!"); }

				// Main Loop
				SDL_Event e;

				bool quit = false;
				while (!quit)
				{
					while (SDL_PollEvent(&e))
					{
						if (e.type == SDL_QUIT) { quit = true; }
					}
				}
			}
		}

		// Cleanup
		vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);
		vkDestroyInstance(vulkanInstance, nullptr);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
}