#include "SplitEngine/Rendering/Vulkan/QueueFamily.hpp"

#include "SplitEngine/Rendering/Vulkan/CommandBuffer.hpp"
#include "SplitEngine/Rendering/Vulkan/Utility.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	QueueFamily::QueueFamily(Device* device, uint32_t familyIndex, uint32_t queueCount) :
		DeviceObject(device),
		_index(familyIndex),
		_queueCount(queueCount)
	{
		for (int i = 0; i < _queueCount; ++i) { _vkQueues.push_back(device->GetVkDevice().getQueue(_index, i)); }

		const vk::CommandPoolCreateInfo graphicsCommandPoolCreateInfo = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _index);

		_commandPool = device->GetVkDevice().createCommandPool(graphicsCommandPoolCreateInfo);
	}

	void QueueFamily::AddSupportedQueueType(QueueType queueType) { _queueTypes.Set(1 << static_cast<uint8_t>(queueType)); }

	void QueueFamily::Destroy() { Utility::DeleteDeviceHandle(GetDevice(), _commandPool); }

	const vk::Queue&       QueueFamily::GetVkQueue(uint32_t index) const { return _vkQueues[index]; }
	const vk::CommandPool& QueueFamily::GetCommandPool() const { return _commandPool; }

	vk::CommandBuffer& QueueFamily::BeginOneshotCommands()
	{
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_commandPool, vk::CommandBufferLevel::ePrimary, 1);

		_currentOneshotCommandBuffer = GetDevice()->GetVkDevice().allocateCommandBuffers(commandBufferAllocateInfo)[0];

		constexpr vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		_currentOneshotCommandBuffer.begin(beginInfo);

		return _currentOneshotCommandBuffer;
	}

	void QueueFamily::EndOneshotCommands(uint32_t index)
	{
		_currentOneshotCommandBuffer.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &_currentOneshotCommandBuffer;

		// TODO: add load balancing on queues
		_vkQueues[index].submit(1, &submitInfo, VK_NULL_HANDLE);
		_vkQueues[index].waitIdle();

		GetDevice()->GetVkDevice().freeCommandBuffers(_commandPool, 1, &_currentOneshotCommandBuffer);
	}

	CommandBuffer QueueFamily::AllocateCommandBuffer(QueueType type, vk::CommandBufferLevel commandBufferLevel, bool singleInstance)
	{
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(_commandPool,
		                                                                                              commandBufferLevel,
		                                                                                              singleInstance ? 1 : Device::MAX_FRAMES_IN_FLIGHT);

		std::vector<vk::CommandBuffer> commandBuffers = GetDevice()->GetVkDevice().allocateCommandBuffers(commandBufferAllocateInfo);
		
		return CommandBuffer(GetDevice(), type, GetDevice()->CreateInFlightResource<vk::CommandBuffer>(std::move(commandBuffers), singleInstance));
	}

	void QueueFamily::DeallocateCommandBuffer(CommandBuffer& commandBuffer) const
	{
		GetDevice()->GetVkDevice().freeCommandBuffers(_commandPool, commandBuffer.GetVkCommandBufferRaw().GetDataVector());
	}
}
