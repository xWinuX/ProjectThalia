#pragma once

#include <SDL2/SDL.h>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Vulkan
{
	struct QueueFamilyIndices
	{
		public:
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			[[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		public:
			vk::SurfaceCapabilitiesKHR        capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR>   presentModes;
	};

	class VulkanContext
	{
		public:
			void Initialize(SDL_Window* sdlWindow);
			void Destroy();

		private:
			const std::vector<const char*> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char*> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			vk::PhysicalDevice _physicalDevice = VK_NULL_HANDLE;
			vk::Device         _device         = VK_NULL_HANDLE;
			vk::Instance       _instance       = VK_NULL_HANDLE;
			vk::SurfaceKHR     _surface        = VK_NULL_HANDLE;
			vk::SwapchainKHR   _swapChain      = VK_NULL_HANDLE;

			vk::SurfaceFormatKHR       _swapChainImageFormat;
			vk::Extent2D               _swapChainExtent;
			std::vector<vk::Image>     _swapChainImages;
			std::vector<vk::ImageView> _swapChainImageViews;

			QueueFamilyIndices      _queueFamilyIndices;
			SwapChainSupportDetails _swapChainSupportDetails;

			void CreateInstance(SDL_Window* sdlWindow);
			void CreateSurface(SDL_Window* sdlWindow);
			void SelectPhysicalDevice();
			void CreateLogicalDevice();
			void CreateSwapChain(SDL_Window* sdlWindow);
			void CreateImageViews();
	};
}
