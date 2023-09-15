#include "ProjectThalia/VulkanContext.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "SDL_vulkan.h"
#include <vector>

namespace ProjectThalia
{
	void VulkanContext::CreateInstance(SDL_Window* sdlWindow, const std::vector<const char*>& validationLayers)
	{
		// Get vulkan instance extensions
		uint32_t extensionCount;
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);

		std::vector<const char*> extensionNames(extensionCount);
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, extensionNames.data());


		// Create application info
		VkApplicationInfo applicationInfo = {.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
											 .pApplicationName   = "Project Thalia",
											 .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
											 .pEngineName        = "No Engine",
											 .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
											 .apiVersion         = VK_API_VERSION_1_3};

		// Create instance info
		VkInstanceCreateInfo createInfo = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
										   .pApplicationInfo        = &applicationInfo,
										   .enabledLayerCount       = static_cast<uint32_t>(validationLayers.size()),
										   .ppEnabledLayerNames     = validationLayers.data(),
										   .enabledExtensionCount   = extensionCount,
										   .ppEnabledExtensionNames = extensionNames.data()};

		// Create vulkan instance
		VkResult instanceCreationResult = vkCreateInstance(&createInfo, nullptr, &_instance);
		if (instanceCreationResult != VK_SUCCESS) { ErrorHandler::ThrowRuntimeError("Failed to create Vulkan instance!"); }
	}

	void VulkanContext::CreateSurface(SDL_Window* sdlWindow)
	{
		SDL_bool surfaceCreationResult = SDL_Vulkan_CreateSurface(sdlWindow, _instance, &_surface);
		if (surfaceCreationResult == SDL_FALSE) { ErrorHandler::ThrowRuntimeError("Failed to create SDL Vulkan surface!"); }
	}

	void VulkanContext::Initialize(SDL_Window* sdlWindow)
	{
		const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
		const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		CreateInstance(sdlWindow, validationLayers);

		CreateSurface(sdlWindow);

		_vulkanDevice = VulkanDevice::FindAndSetupBestDevice(_instance, _surface, validationLayers, deviceExtensions);

		// Create Present Queue
		VkQueue presentQueue;
		vkGetDeviceQueue(_vulkanDevice.GetDevice(), _vulkanDevice.GetQueueFamilyIndices().presentFamily.value(), 0, &presentQueue);
	}

	void VulkanContext::Destroy()
	{
		vkDestroyDevice(_vulkanDevice.GetDevice(), nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
	}

}