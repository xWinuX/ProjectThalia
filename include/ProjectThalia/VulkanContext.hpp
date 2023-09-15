#pragma once

#include "VulkanDevice.hpp"
#include "vulkan/vulkan.h"
#include <SDL2/SDL.h>
#include <optional>
#include <vector>

namespace ProjectThalia
{
	class VulkanContext
	{
		public:
			void Initialize(SDL_Window* sdlWindow);
			void Destroy();

		private:
			struct SwapChainSupportDetails
			{
					VkSurfaceCapabilitiesKHR        capabilities;
					std::vector<VkSurfaceFormatKHR> formats;
					std::vector<VkPresentModeKHR>   presentModes;
			};

			VulkanDevice _vulkanDevice;
			VkInstance   _instance = VK_NULL_HANDLE;
			VkSurfaceKHR _surface  = VK_NULL_HANDLE;

			void CreateInstance(SDL_Window* sdlWindow, const std::vector<const char*>& validationLayers);
			void CreateSurface(SDL_Window* sdlWindow);
	};
}
