#include "ProjectThalia/Rendering/Vulkan/Device.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include <set>

namespace ProjectThalia::Rendering::Vulkan
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

		_memoryProperties = _physicalDevice.GetVkPhysicalDevice().getMemoryProperties();
		_graphicsQueue    = _vkDevice.getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
		_presentQueue     = _vkDevice.getQueue(queueFamilyIndices.presentFamily.value(), 0);
	}

	void Device::CreateRenderPass() { _renderPass = RenderPass(this); }

	void Device::CreateSwapchain(vk::SurfaceKHR surfaceKhr, glm::ivec2 size)
	{
		// Destroy previously created swapchain (since the swapchain can be recreated when the window is resized this)
		_swapchain.Destroy();

		_swapchain = Swapchain(this, surfaceKhr, {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)});
	}

	void Device::CreatePipeline(const std::string& name, std::vector<Pipeline::ShaderInfo> shaderInfos) { _pipeline = Pipeline(this, name, shaderInfos); }

	const vk::Device& Device::GetVkDevice() const { return _vkDevice; }

	const PhysicalDevice& Device::GetPhysicalDevice() const { return _physicalDevice; }

	const vk::Queue& Device::GetGraphicsQueue() const { return _graphicsQueue; }

	const vk::Queue& Device::GetPresentQueue() const { return _presentQueue; }

	const RenderPass& Device::GetRenderPass() const { return _renderPass; }

	const Swapchain& Device::GetSwapchain() const { return _swapchain; }

	const Pipeline& Device::GetPipeline() const { return _pipeline; }

	const vk::PhysicalDeviceMemoryProperties& Device::GetMemoryProperties() const { return _memoryProperties; }

	void Device::Destroy()
	{
		_swapchain.Destroy();
		_renderPass.Destroy();
		_pipeline.Destroy();
		_vkDevice.destroy(_graphicsCommandPool);
		_vkDevice.destroy();
	}

	void Device::CreateGraphicsCommandPool()
	{
		vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
																					_physicalDevice.GetQueueFamilyIndices().graphicsFamily.value());

		_graphicsCommandPool = _vkDevice.createCommandPool(commandPoolCreateInfo);
	}

	const vk::CommandPool& Device::GetGraphicsCommandPool() const { return _graphicsCommandPool; }

}
