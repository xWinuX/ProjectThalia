#include "ProjectThalia/VulkanContext.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "SDL2/SDL_vulkan.h"
#include <format>
#include <set>
#include <vector>

namespace ProjectThalia::Vulkan
{
	void VulkanContext::Initialize(SDL_Window* sdlWindow)
	{
		CreateInstance(sdlWindow);

		CreateSurface(sdlWindow);

		SelectPhysicalDevice();

		CreateLogicalDevice();

		// Create Present Queue
		VkQueue presentQueue;
		vkGetDeviceQueue(_device, _queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	}

	void VulkanContext::CreateInstance(SDL_Window* sdlWindow)
	{
		// Get vulkan instance extensions
		uint32_t extensionCount;
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);

		std::vector<const char*> extensionNames = std::vector<const char*>(extensionCount, nullptr);
		SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, extensionNames.data());

		// Create infos
		vk::ApplicationInfo    applicationInfo    = vk::ApplicationInfo("Project Thalia", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);
		vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo({}, &applicationInfo, _validationLayers, extensionNames);

		// Create vulkan instance
		vk::Result instanceCreationResult = vk::createInstance(&instanceCreateInfo, nullptr, &_instance);
		if (instanceCreationResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create Vulkan instance!"); }
	}

	void VulkanContext::CreateSurface(SDL_Window* sdlWindow)
	{
		// Create surface from sdl
		VkSurfaceKHR surfaceHandle         = VK_NULL_HANDLE;
		SDL_bool     surfaceCreationResult = SDL_Vulkan_CreateSurface(sdlWindow, static_cast<VkInstance>(_instance), &surfaceHandle);
		_surface                           = surfaceHandle;
		if (surfaceCreationResult == SDL_FALSE) { ErrorHandler::ThrowRuntimeError("Failed to create SDL Vulkan surface!"); }
	}

	void VulkanContext::SelectPhysicalDevice()
	{
		std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
		for (const vk::PhysicalDevice& physicalDevice : physicalDevices)
		{
			// Check device type
			vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
			if (deviceProperties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) { continue; }

			// Check Extensions
			std::vector<vk::ExtensionProperties, std::allocator<vk::ExtensionProperties>> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

			std::set<std::string> requiredExtensions = std::set<std::string>(_deviceExtensions.begin(), _deviceExtensions.end());

			for (const auto& extension : availableExtensions)
			{
				requiredExtensions.erase(extension.extensionName);
			}

			if (!requiredExtensions.empty()) { continue; }

			// Check queues
			_queueFamilyIndices        = QueueFamilyIndices();
			auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
			int  i                     = 0;
			for (const vk::QueueFamilyProperties& queueFamily : queueFamilyProperties)
			{
				unsigned int presentSupport = physicalDevice.getSurfaceSupportKHR(i, _surface);

				if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && !_queueFamilyIndices.graphicsFamily.has_value())
				{
					Debug::Log::Info(std::format("graphics index: {}", i));
					_queueFamilyIndices.graphicsFamily = i;
				}

				if (presentSupport && !_queueFamilyIndices.presentFamily.has_value())
				{
					Debug::Log::Info(std::format("present index: {}", i));
					_queueFamilyIndices.presentFamily = i;
				}

				if (_queueFamilyIndices.isComplete()) { break; }

				i++;
			}

			if (!_queueFamilyIndices.isComplete()) { continue; }

			// Select device
			_physicalDevice = physicalDevice;
			Debug::Log::Info(std::format("Selected physical device: {}", deviceProperties.deviceName));
			break;
		}

		// Check if we found a compatible GPU
		if (_physicalDevice == VK_NULL_HANDLE) { ErrorHandler::ThrowRuntimeError("This device does not have any gpus meeting the applications requirements"); }
	}

	void VulkanContext::CreateLogicalDevice()
	{
		// Get queue info
		std::set<uint32_t> uniqueQueueFamilies = {_queueFamilyIndices.graphicsFamily.value(),
												  _queueFamilyIndices.presentFamily.value()}; // Needs to be a set to filter out same queue features

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>(uniqueQueueFamilies.size());

		float queuePriority = 1.0f;

		int i = 0;
		for (const auto& uniqueQueueFamily : uniqueQueueFamilies)
		{
			queueCreateInfos[i] = vk::DeviceQueueCreateInfo({}, uniqueQueueFamily, 1, &queuePriority);
			i++;
		}

		// Create logical device
		vk::PhysicalDeviceFeatures deviceFeatures   = _physicalDevice.getFeatures();
		vk::DeviceCreateInfo       deviceCreateInfo = vk::DeviceCreateInfo({}, queueCreateInfos, _validationLayers, _deviceExtensions, &deviceFeatures);

		vk::Result vulkanDeviceCreateResult = _physicalDevice.createDevice(&deviceCreateInfo, nullptr, &_device);
		if (vulkanDeviceCreateResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create logical device!"); }
	}

	void VulkanContext::Destroy()
	{
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
	}
}