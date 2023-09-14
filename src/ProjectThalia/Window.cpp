#include "ProjectThalia/Window.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"

#include <format>
#include <stdexcept>
#include <vector>

namespace ProjectThalia
{
	void Window::Open()
	{
		const int SCREEN_WIDTH  = 500;
		const int SCREEN_HEIGHT = 500;

		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) { ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {}\n", SDL_GetError())); }
		else
		{
			// Create _window
			_window = SDL_CreateWindow("Project Thalia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
			if (_window == nullptr) { ErrorHandler::ThrowRuntimeError(std::format("Window could not be created! SDL_Error: {}\n", SDL_GetError())); }
			else
			{
				// Get vulkan instance extensions
				uint32_t extensionCount;
				SDL_Vulkan_GetInstanceExtensions(_window, &extensionCount, nullptr);

				std::vector<const char*> extensionNames(extensionCount);
				SDL_Vulkan_GetInstanceExtensions(_window, &extensionCount, extensionNames.data());

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
				VkResult instanceCreationResult = vkCreateInstance(&createInfo, nullptr, &_vulkanInstance);
				if (instanceCreationResult != VK_SUCCESS) { ErrorHandler::ThrowRuntimeError("failed to create Vulkan instance!"); }

				// Create vulkan surface
				SDL_bool surfaceCreationResult = SDL_Vulkan_CreateSurface(_window, _vulkanInstance, &_vulkanSurface);
				if (surfaceCreationResult == SDL_FALSE) { ErrorHandler::ThrowRuntimeError("failed to create SDL Vulkan _vulkanSurface!"); }

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
		vkDestroySurfaceKHR(_vulkanInstance, _vulkanSurface, nullptr);
		vkDestroyInstance(_vulkanInstance, nullptr);
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}
}