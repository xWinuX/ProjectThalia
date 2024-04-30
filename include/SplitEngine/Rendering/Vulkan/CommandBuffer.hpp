#pragma once
#include "InFlightResource.hpp"
#include "DeviceObject.hpp"
#include "QueueType.hpp"

#include <vulkan/vulkan.hpp>


namespace SplitEngine::Rendering::Vulkan
{
	class CommandBuffer : public DeviceObject
	{
		public:
			CommandBuffer() = default;
			CommandBuffer(Device* device, QueueType type, InFlightResource<vk::CommandBuffer>&& commandBuffers);

			vk::CommandBuffer& GetVkCommandBuffer(const uint32_t fifIndex = -1);

			InFlightResource<vk::CommandBuffer>& GetVkCommandBufferRaw();

			void Destroy() override;

		private:
			QueueType                           _queueType = QueueType::MAX_VALUE;
			InFlightResource<vk::CommandBuffer> _commandBuffers;
	};
}
