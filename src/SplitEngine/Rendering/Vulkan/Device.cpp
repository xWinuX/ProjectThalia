#include "SplitEngine/Rendering/Vulkan/Device.hpp"
#include <set>
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Rendering/Vulkan/Instance.hpp"
#include "SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	Device::Device(PhysicalDevice& physicalDevice):
		_physicalDevice(physicalDevice)
	{
		CreateLogicalDevice(physicalDevice);

		CreateCommandPools(physicalDevice);

		CreateCommandBuffers();

		CreateRenderPass();

		CreateSyncObjects();
	}


	void Device::CreateLogicalDevice(PhysicalDevice& physicalDevice)
	{
		// Get queue info
		const PhysicalDevice::QueueFamilyIndices queueFamilyIndices  = physicalDevice.GetQueueFamilyIndices();
		std::set<uint32_t>                       uniqueQueueFamilies = { queueFamilyIndices.GraphicsFamily.value(), queueFamilyIndices.PresentFamily.value() };
		// Needs to be a set to filter out same queue features

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>(uniqueQueueFamilies.size());

		float queuePriority = 1.0f;

		int i = 0;
		for (const auto& uniqueQueueFamily: uniqueQueueFamilies)
		{
			queueCreateInfos[i] = vk::DeviceQueueCreateInfo({}, uniqueQueueFamily, 1, &queuePriority);
			i++;
		}

		// Create logical device
		vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.GetVkPhysicalDevice().getFeatures();
		vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo({}, queueCreateInfos, physicalDevice.GetValidationLayers(), physicalDevice.GetExtensions(), &deviceFeatures);

		vk::Result vulkanDeviceCreateResult = physicalDevice.GetVkPhysicalDevice().createDevice(&deviceCreateInfo, nullptr, &_vkDevice);
		if (vulkanDeviceCreateResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create logical device!"); }

		_memoryProperties = physicalDevice.GetVkPhysicalDevice().getMemoryProperties();
		_graphicsQueue    = _vkDevice.getQueue(queueFamilyIndices.GraphicsFamily.value(), 0);
		_presentQueue     = _vkDevice.getQueue(queueFamilyIndices.PresentFamily.value(), 0);
		_computeQueue     = _vkDevice.getQueue(queueFamilyIndices.ComputeFamily.value(), 0);
	}

	void Device::CreateSyncObjects()
	{
		constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
		constexpr vk::FenceCreateInfo     fenceCreateInfo     = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

		std::vector<vk::Semaphore> imageAvailableSemaphores;
		std::vector<vk::Semaphore> renderingFinishedSemaphores;
		std::vector<vk::Fence>     inFlightFences;

		for (int i = 0; i < Device::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			imageAvailableSemaphores.push_back(_vkDevice.createSemaphore(semaphoreCreateInfo));
			renderingFinishedSemaphores.push_back(_vkDevice.createSemaphore(semaphoreCreateInfo));
			inFlightFences.push_back(_vkDevice.createFence(fenceCreateInfo));
		}

		_imageAvailableSemaphore = InFlightResource<vk::Semaphore>(GetCurrentFramePtr(), std::move(imageAvailableSemaphores));
		_renderFinishedSemaphore = InFlightResource<vk::Semaphore>(GetCurrentFramePtr(), std::move(renderingFinishedSemaphores));
		_inFlightFence           = InFlightResource<vk::Fence>(GetCurrentFramePtr(), std::move(inFlightFences));
	}

	void Device::CreateCommandBuffers()
	{
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_graphicsCommandPool,
		                                                                                              vk::CommandBufferLevel::ePrimary,
		                                                                                              Device::MAX_FRAMES_IN_FLIGHT);

		std::vector<vk::CommandBuffer> commandBuffers = _vkDevice.allocateCommandBuffers(commandBufferAllocateInfo);
		_graphicsCommandBuffer                        = InFlightResource<vk::CommandBuffer>(GetCurrentFramePtr(), std::move(commandBuffers));
	}

	void Device::CreateRenderPass() { _renderPass = RenderPass(this); }

	void Device::CreateSwapchain(vk::SurfaceKHR surfaceKhr, glm::ivec2 size)
	{
		// Destroy previously created swapchain (since the swapchain can be recreated when the window is resized this)
		if (_swapchain) { _swapchain->Destroy(); }
		_physicalDevice.UpdateSwapchainSupportDetails(surfaceKhr);
		_swapchain.reset(new Swapchain(this, surfaceKhr, { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) }));
	}

	void Device::DestroySwapchain() { _swapchain->Destroy(); }

	void Device::CreateCommandPools(PhysicalDevice& physicalDevice)
	{
		const vk::CommandPoolCreateInfo graphicsCommandPoolCreateInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		                                                                                          physicalDevice.GetQueueFamilyIndices().GraphicsFamily.value());

		_graphicsCommandPool = _vkDevice.createCommandPool(graphicsCommandPoolCreateInfo);

		const vk::CommandPoolCreateInfo computeCommandPoolCreateInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		                                                                                         physicalDevice.GetQueueFamilyIndices().ComputeFamily.value());
		_computeCommandPool = _vkDevice.createCommandPool(computeCommandPoolCreateInfo);
	}


	const vk::Device& Device::GetVkDevice() const { return _vkDevice; }

	const vk::Queue& Device::GetGraphicsQueue() const { return _graphicsQueue; }
	const vk::Queue& Device::GetPresentQueue() const { return _presentQueue; }
	const vk::Queue& Device::GetComputeQueue() const { return _computeQueue; }

	const RenderPass& Device::GetRenderPass() const { return _renderPass; }

	const Swapchain& Device::GetSwapchain() const { return *_swapchain.get(); }

	const vk::CommandPool& Device::GetGraphicsCommandPool() const { return _graphicsCommandPool; }

	const vk::PhysicalDeviceMemoryProperties& Device::GetMemoryProperties() const { return _memoryProperties; }

	const vk::Semaphore& Device::GetImageAvailableSemaphore() const { return _imageAvailableSemaphore.Get(); }

	const vk::Semaphore& Device::GetRenderFinishedSemaphore() const { return _renderFinishedSemaphore.Get(); }

	const vk::Fence&         Device::GetInFlightFence() const { return _inFlightFence.Get(); }
	const vk::CommandBuffer& Device::GetGraphicsCommandBuffer() const { return _graphicsCommandBuffer.Get(); }
	const vk::CommandBuffer& Device::GetComputeCommandBuffer() const { return _computeCommandBuffer.Get(); }

	void Device::Destroy()
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			_vkDevice.destroy(_imageAvailableSemaphore[i]);
			_vkDevice.destroy(_renderFinishedSemaphore[i]);
			_vkDevice.destroy(_inFlightFence[i]);
		}

		_renderPass.Destroy();

		_vkDevice.destroy(_graphicsCommandPool);

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
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1);

		const vk::CommandBuffer commandBuffer = _vkDevice.allocateCommandBuffers(commandBufferAllocateInfo)[0];

		constexpr vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void Device::EndOneshotCommands(const vk::CommandBuffer commandBuffer) const
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &commandBuffer;

		_graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
		_graphicsQueue.waitIdle();

		_vkDevice.freeCommandBuffers(_graphicsCommandPool, 1, &commandBuffer);
	}

	uint32_t* Device::GetCurrentFramePtr() { return &_currentFrame; }

	void Device::AdvanceFrame() { _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; }

	void Device::WaitForIdle() { _vkDevice.waitIdle(); }

	const PhysicalDevice& Device::GetPhysicalDevice() const { return _physicalDevice; }
}
