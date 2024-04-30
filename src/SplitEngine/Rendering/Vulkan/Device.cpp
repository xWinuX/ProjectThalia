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

		CreateRenderPass();

		CreateSyncObjects();
	}

	void Device::CreateLogicalDevice(PhysicalDevice& physicalDevice)
	{
		// Get queue info
		const PhysicalDevice::QueueFamilyInfos queueFamilyIndices = physicalDevice.GetQueueFamilyInfos();

		// Get unique queue families
		std::vector<PhysicalDevice::QueueFamilyInfo> sortedQueueFamilies = std::vector(queueFamilyIndices.begin(), queueFamilyIndices.end());
		std::ranges::sort(sortedQueueFamilies,
		                  [](const PhysicalDevice::QueueFamilyInfo& queueFamilyInfo1, const PhysicalDevice::QueueFamilyInfo& queueFamilyInfo2)
		                  {
			                  return queueFamilyInfo1.Index > queueFamilyInfo2.Index;
		                  });

		// Needs to be a set to filter out same queue features
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		std::vector<std::vector<float>> prioritiesList = std::vector<std::vector<float>>();

		uint32_t previousIndex = -1;
		for (const auto& queueFamily: sortedQueueFamilies)
		{
			if (previousIndex == queueFamily.Index) { continue; }
			prioritiesList.emplace_back(queueFamily.QueueCount, 1.0f);
			for (int i = prioritiesList.back().size() - 1; i >= 0; --i) { prioritiesList.back()[i] = static_cast<float>(i) * (1.0f / static_cast<float>(queueFamily.QueueCount)); }

			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo({}, queueFamily.Index, queueFamily.QueueCount, prioritiesList.back().data()));
			previousIndex = queueFamily.Index;
		}

		// Create logical device
		vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.GetVkPhysicalDevice().getFeatures();
		vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo({}, queueCreateInfos, physicalDevice.GetValidationLayers(), physicalDevice.GetExtensions(), &deviceFeatures);

		vk::Result vulkanDeviceCreateResult = physicalDevice.GetVkPhysicalDevice().createDevice(&deviceCreateInfo, nullptr, &_vkDevice);
		if (vulkanDeviceCreateResult != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create logical device!"); }

		_memoryProperties = physicalDevice.GetVkPhysicalDevice().getMemoryProperties();

		previousIndex = -1;
		for (auto& queueFamily: sortedQueueFamilies)
		{
			if (previousIndex == queueFamily.Index)
			{
				_queueFamilyLookup[static_cast<size_t>(queueFamily.WantedCommandType)] = _queueFamilies.size() - 1;
				_queueFamilies.back().AddSupportedQueueType(queueFamily.WantedCommandType);
				continue;
			}

			_queueFamilies.emplace_back(this, queueFamily.Index, queueFamily.QueueCount);
			_queueFamilies.back().AddSupportedQueueType(queueFamily.WantedCommandType);
			_queueFamilyLookup[static_cast<size_t>(queueFamily.WantedCommandType)] = _queueFamilies.size() - 1;

			previousIndex = queueFamily.Index;
		}
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

	void Device::CreateRenderPass() { _renderPass = RenderPass(this); }

	void Device::CreateSwapchain(vk::SurfaceKHR surfaceKhr, glm::ivec2 size)
	{
		// Destroy previously created swapchain (since the swapchain can be recreated when the window is resized this)
		if (_swapchain) { _swapchain->Destroy(); }
		_physicalDevice.UpdateSwapchainSupportDetails(surfaceKhr);
		_swapchain.reset(new Swapchain(this, surfaceKhr, { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) }));
	}

	void Device::DestroySwapchain() { _swapchain->Destroy(); }

	const vk::Device& Device::GetVkDevice() const { return _vkDevice; }

	const QueueFamily& Device::GetQueueFamily(const QueueType queueFamilyType) const { return _queueFamilies[_queueFamilyLookup[static_cast<size_t>(queueFamilyType)]]; }

	QueueFamily& Device::GetQueueFamily(const QueueType queueFamilyType) { return _queueFamilies[_queueFamilyLookup[static_cast<size_t>(queueFamilyType)]]; }


	const RenderPass& Device::GetRenderPass() const { return _renderPass; }

	const Swapchain& Device::GetSwapchain() const { return *_swapchain.get(); }


	const vk::PhysicalDeviceMemoryProperties& Device::GetMemoryProperties() const { return _memoryProperties; }

	const vk::Semaphore& Device::GetImageAvailableSemaphore() const { return _imageAvailableSemaphore.Get(); }

	const vk::Semaphore& Device::GetRenderFinishedSemaphore() const { return _renderFinishedSemaphore.Get(); }

	const vk::Fence& Device::GetInFlightFence() const { return _inFlightFence.Get(); }

	void Device::Destroy()
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			_vkDevice.destroy(_imageAvailableSemaphore[i]);
			_vkDevice.destroy(_renderFinishedSemaphore[i]);
			_vkDevice.destroy(_inFlightFence[i]);
		}

		for (QueueFamily& queueFamily: _queueFamilies) { queueFamily.Destroy(); }

		_renderPass.Destroy();

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

	uint32_t* Device::GetCurrentFramePtr(const bool staticPtr) { return staticPtr ? &_currentStaticFrame : &_currentFrame; }

	void Device::AdvanceFrame() { _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; }

	void Device::WaitForIdle() { _vkDevice.waitIdle(); }

	const PhysicalDevice& Device::GetPhysicalDevice() const { return _physicalDevice; }
}
