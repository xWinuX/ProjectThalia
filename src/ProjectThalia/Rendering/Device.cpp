#include "ProjectThalia/Rendering/Device.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include <set>

namespace ProjectThalia::Rendering
{
	Device::Device(PhysicalDevice& physicalDevice) :
		_physicalDevice(physicalDevice)
	{
		// Get queue info
		const PhysicalDevice::QueueFamilyIndices queueFamilyIndices  = physicalDevice.GetQueueFamilyIndices();
		std::set<uint32_t>                       uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(),
																		queueFamilyIndices.presentFamily.value()}; // Needs to be a set to filter out same queue features

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>(uniqueQueueFamilies.size());

		float queuePriority = 1.0f;

		int i = 0;
		for (const auto& uniqueQueueFamily : uniqueQueueFamilies)
		{
			queueCreateInfos[i] = vk::DeviceQueueCreateInfo({}, uniqueQueueFamily, 1, &queuePriority);
			i++;
		}

		// Create logical device
		vk::PhysicalDeviceFeatures deviceFeatures   = _physicalDevice.GetVkPhysicalDevice().getFeatures();
		vk::DeviceCreateInfo       deviceCreateInfo = vk::DeviceCreateInfo({},
                                                                     queueCreateInfos,
                                                                     physicalDevice.GetValidationLayers(),
                                                                     physicalDevice.GetExtensions(),
                                                                     &deviceFeatures);

		vk::Result vulkanDeviceCreateResult = _physicalDevice.GetVkPhysicalDevice().createDevice(&deviceCreateInfo, nullptr, &_vkDevice);
		if (vulkanDeviceCreateResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create logical device!"); }

		_graphicsQueue = _vkDevice.getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
		_presentQueue  = _vkDevice.getQueue(queueFamilyIndices.presentFamily.value(), 0);
	}

	const vk::Device& Device::GetVkDevice() const { return _vkDevice; }

	const PhysicalDevice& Device::GetPhysicalDevice() const { return _physicalDevice; }

	const vk::Queue& Device::GetGraphicsQueue() const { return _graphicsQueue; }

	const vk::Queue& Device::GetPresentQueue() const { return _presentQueue; }
}
