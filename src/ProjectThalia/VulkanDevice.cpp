#include "ProjectThalia/VulkanDevice.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include <set>

namespace ProjectThalia
{
	VulkanDevice VulkanDevice::FindAndSetupBestDevice(const VkInstance&               vulkanInstance,
													  const VkSurfaceKHR&             surfaceKhr,
													  const std::vector<const char*>& validationLayers,
													  const std::vector<const char*>& deviceExtensions)
	{
		VkPhysicalDevice   finalPhysicalDevice = VK_NULL_HANDLE;
		VkDevice           device              = VK_NULL_HANDLE;
		QueueFamilyIndices queueFamilyIndices;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);

		if (deviceCount == 0) { ErrorHandler::ThrowRuntimeError("Failed to find GPUS that are compatible with vulkan"); }

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

		// Check if device is suitable
		for (const VkPhysicalDevice& physicalDevice : devices)
		{
			// Check device type
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
			if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { continue; }

			// Check Extensions
			bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice, deviceExtensions);
			if (!extensionsSupported) { continue; }

			// Check queues
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
			queueFamilyIndices = FindQueueFamilies(physicalDevice, surfaceKhr, queueFamilies);
			if (!queueFamilyIndices.isComplete()) { continue; }

			finalPhysicalDevice = physicalDevice;
			break;
		}

		// Check if we found a compatible GPU
		if (finalPhysicalDevice == VK_NULL_HANDLE) { ErrorHandler::ThrowRuntimeError("This Device does not have any GPU meeting the applications requirements"); }

		// Get queue info
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t>                   uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
													   .queueFamilyIndex = queueFamily,
													   .queueCount       = 1,
													   .pQueuePriorities = &queuePriority};

			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Create logical device
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(finalPhysicalDevice, &deviceFeatures);
		VkDeviceCreateInfo deviceCreateInfo = {.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
											   .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
											   .pQueueCreateInfos       = queueCreateInfos.data(),
											   .enabledLayerCount       = static_cast<uint32_t>(validationLayers.size()),
											   .ppEnabledLayerNames     = validationLayers.data(),
											   .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
											   .ppEnabledExtensionNames = deviceExtensions.data(),
											   .pEnabledFeatures        = &deviceFeatures};

		VkResult vulkanDeviceCreateResult = vkCreateDevice(finalPhysicalDevice, &deviceCreateInfo, nullptr, &device);
		if (vulkanDeviceCreateResult != VK_SUCCESS) { ErrorHandler::ThrowRuntimeError("Failed to create logical device!"); }

		return {finalPhysicalDevice, device, queueFamilyIndices};
	}


	VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilyIndices queueFamilyIndices) :
		_physicalDevice(physicalDevice),
		_device(device),
		_queueFamilyIndices(queueFamilyIndices)
	{
	}

	bool VulkanDevice::IsDeviceSuitable(const VkPhysicalDevice& physicalDevice, const std::vector<const char*>& extensions)
	{
		// Check device type
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { return false; }

		// Check Extensions
		bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice, extensions);
		if (!extensionsSupported) { return false; }

		return true;
	}

	bool VulkanDevice::CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& deviceExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	VulkanDevice::QueueFamilyIndices
	VulkanDevice::FindQueueFamilies(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const std::vector<VkQueueFamilyProperties>& queueFamilies)
	{
		QueueFamilyIndices indices;

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.graphicsFamily = i; }

			if (presentSupport) { indices.presentFamily = i; }

			if (indices.isComplete()) { break; }

			i++;
		}
		return indices;
	}

	const VkPhysicalDevice&                 VulkanDevice::GetPhysicalDevice() const { return _physicalDevice; }
	const VkDevice&                         VulkanDevice::GetDevice() const { return _device; }
	const VulkanDevice::QueueFamilyIndices& VulkanDevice::GetQueueFamilyIndices() const { return _queueFamilyIndices; }

}