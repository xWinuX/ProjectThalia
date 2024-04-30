#include "SplitEngine/Rendering/Vulkan/CommandBuffer.hpp"

#include "SplitEngine/Rendering/Vulkan/Device.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	CommandBuffer::CommandBuffer(Device* device, QueueType type, InFlightResource<vk::CommandBuffer>&& commandBuffers):
		DeviceObject(device),
		_commandBuffers(std::move(commandBuffers)),
		_queueType(type) {}

	vk::CommandBuffer&                   CommandBuffer::GetVkCommandBuffer(const uint32_t fifIndex) { return fifIndex == -1 ? _commandBuffers.Get() : _commandBuffers[fifIndex]; }
	InFlightResource<vk::CommandBuffer>& CommandBuffer::GetVkCommandBufferRaw() { return _commandBuffers; }

	void CommandBuffer::Destroy() { GetDevice()->GetQueueFamily(_queueType).DeallocateCommandBuffer(*this); }
}
