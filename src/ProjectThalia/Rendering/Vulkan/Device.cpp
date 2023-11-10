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
		std::set<uint32_t>                       uniqueQueueFamilies = {queueFamilyIndices.GraphicsFamily.value(),
																		queueFamilyIndices.PresentFamily.value()}; // Needs to be a set to filter out same queue features

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
		_graphicsQueue    = _vkDevice.getQueue(queueFamilyIndices.GraphicsFamily.value(), 0);
		_presentQueue     = _vkDevice.getQueue(queueFamilyIndices.PresentFamily.value(), 0);
	}

	void Device::CreateRenderPass() { _renderPass = RenderPass(this); }

	void Device::CreateSwapchain(vk::SurfaceKHR surfaceKhr, glm::ivec2 size)
	{
		// Destroy previously created swapchain (since the swapchain can be recreated when the window is resized this)
		_swapchain.Destroy();

		_swapchain = Swapchain(this, surfaceKhr, {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)});
	}

	void Device::CreatePipeline(const std::string&                             name,
								const std::vector<Pipeline::ShaderInfo>&       shaderInfos,
								const vk::ArrayProxy<vk::DescriptorSetLayout>& uniformBuffers)
	{
		_pipeline = Pipeline(this, name, shaderInfos, uniformBuffers);
	}

	void Device::CreateGraphicsCommandPool()
	{
		vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
																					_physicalDevice.GetQueueFamilyIndices().GraphicsFamily.value());

		_graphicsCommandPool = _vkDevice.createCommandPool(commandPoolCreateInfo);
	}

	void Device::CreateAllocator(const Instance& instance, const AllocatorCreateInfo& allocatorCreateInfo)
	{
		_allocator = Allocator(this, instance, allocatorCreateInfo);
	}

	const vk::Device& Device::GetVkDevice() const { return _vkDevice; }

	const PhysicalDevice& Device::GetPhysicalDevice() const { return _physicalDevice; }

	const vk::Queue& Device::GetGraphicsQueue() const { return _graphicsQueue; }

	const vk::Queue& Device::GetPresentQueue() const { return _presentQueue; }

	const RenderPass& Device::GetRenderPass() const { return _renderPass; }

	const Swapchain& Device::GetSwapchain() const { return _swapchain; }

	const Pipeline& Device::GetPipeline() const { return _pipeline; }

	const vk::CommandPool& Device::GetGraphicsCommandPool() const { return _graphicsCommandPool; }

	Allocator& Device::GetAllocator() { return _allocator; }

	const vk::PhysicalDeviceMemoryProperties& Device::GetMemoryProperties() const { return _memoryProperties; }

	void Device::Destroy()
	{
		_swapchain.Destroy();
		_renderPass.Destroy();
		_pipeline.Destroy();
		_vkDevice.destroy(_graphicsCommandPool);
		_allocator.Destroy();
		_vkDevice.destroy();
	}

	int Device::FindMemoryTypeIndex(const vk::MemoryRequirements& memoryRequirements, const vk::Flags<vk::MemoryPropertyFlagBits>& memoryPropertyFlags) const
	{
		int memoryType = -1;
		for (int i = 0; i < _memoryProperties.memoryTypeCount; i++)
		{
			if ((memoryRequirements.memoryTypeBits & (1 << i)) && (_memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
			{
				memoryType = i;
				break;
			}
		}
		if (memoryType == -1) { ErrorHandler::ThrowRuntimeError("Failed to find suitable memory type!"); }

		return memoryType;
	}

	vk::CommandBuffer Device::BeginOneshotCommands() const
	{
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1);

		vk::CommandBuffer commandBuffer = _vkDevice.allocateCommandBuffers(commandBufferAllocateInfo)[0];

		vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void Device::EndOneshotCommands(vk::CommandBuffer commandBuffer) const
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &commandBuffer;

		_graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
		_graphicsQueue.waitIdle();

		_vkDevice.freeCommandBuffers(_graphicsCommandPool, 1, &commandBuffer);
	}


}
