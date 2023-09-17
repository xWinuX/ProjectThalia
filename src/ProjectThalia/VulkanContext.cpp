#include "ProjectThalia/VulkanContext.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"

#include <SDL2/SDL_vulkan.h>
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

		CreateSwapChain(sdlWindow);

		CreateImageViews();
	}
	void VulkanContext::CreateImageViews()
	{
		_swapChainImageViews.resize(_swapChainImages.size());

		for (int i = 0; i < _swapChainImages.size(); ++i)
		{
			vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo({},
																				  _swapChainImages[i],
																				  vk::ImageViewType::e2D,
																				  _swapChainImageFormat.format,
																				  {vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity},
																				  {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

			_swapChainImageViews[i] = _device.createImageView(imageViewCreateInfo);
		}
	}

	void VulkanContext::CreateSwapChain(SDL_Window* sdlWindow)
	{
		// Select surface format
		_swapChainImageFormat = _swapChainSupportDetails.formats[0];
		for (const auto& availableFormat : _swapChainSupportDetails.formats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) { _swapChainImageFormat = availableFormat; }
		}

		// Select present mode
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		for (const auto& availablePresentMode : _swapChainSupportDetails.presentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			{
				presentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
		}

		// Select swap extend
		const vk::SurfaceCapabilitiesKHR& capabilities = _swapChainSupportDetails.capabilities;
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) { _swapChainExtent = capabilities.currentExtent; }
		else
		{
			int width, height;

			SDL_GetWindowSize(sdlWindow, &width, &height);

			_swapChainExtent = vk::Extent2D(width, height);

			_swapChainExtent.width  = std::clamp(_swapChainExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			_swapChainExtent.height = std::clamp(_swapChainExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		// Select image count
		uint32_t imageCount = _swapChainSupportDetails.capabilities.minImageCount + 1;
		if (_swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > _swapChainSupportDetails.capabilities.maxImageCount)
		{
			imageCount = _swapChainSupportDetails.capabilities.maxImageCount;
		}

		// Create swap chain
		vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR({},
																					_surface,
																					imageCount,
																					_swapChainImageFormat.format,
																					_swapChainImageFormat.colorSpace,
																					_swapChainExtent,
																					1,
																					vk::ImageUsageFlagBits::eColorAttachment);

		if (_queueFamilyIndices.graphicsFamily != _queueFamilyIndices.presentFamily)
		{
			std::vector<uint32_t> queueFamilies = {_queueFamilyIndices.graphicsFamily.value(), _queueFamilyIndices.presentFamily.value()};
			swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
			swapChainCreateInfo.setQueueFamilyIndices(queueFamilies);
		}
		else { swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive); }

		swapChainCreateInfo.setPreTransform(_swapChainSupportDetails.capabilities.currentTransform);
		swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		swapChainCreateInfo.setPresentMode(presentMode);
		swapChainCreateInfo.setClipped(vk::True);
		swapChainCreateInfo.setOldSwapchain(VK_NULL_HANDLE);

		_swapChain = _device.createSwapchainKHR(swapChainCreateInfo);

		_swapChainImages = _device.getSwapchainImagesKHR(_swapChain);
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


			// Check swap chain support
			_swapChainSupportDetails = SwapChainSupportDetails();

			_swapChainSupportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(_surface);
			_swapChainSupportDetails.formats      = physicalDevice.getSurfaceFormatsKHR(_surface);
			_swapChainSupportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(_surface);

			if (_swapChainSupportDetails.formats.empty() || _swapChainSupportDetails.presentModes.empty()) { continue; }

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
		_device.destroy(_swapChain);
		for (const vk::ImageView& imageView : _swapChainImageViews) {
			_device.destroy(imageView);
		}
		_device.destroy();
		_instance.destroy(_surface);
		_instance.destroy();
	}

}