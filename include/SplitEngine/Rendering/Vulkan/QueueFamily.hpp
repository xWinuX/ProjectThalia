#pragma once

#include "DeviceObject.hpp"
#include "QueueType.hpp"

#include <vulkan/vulkan.hpp>

#include "CommandBuffer.hpp"
#include "InFlightResource.hpp"
#include "SplitEngine/DataStructures.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	class QueueFamily final : public DeviceObject
	{
		friend CommandBuffer;

		public:
			QueueFamily() = default;

			QueueFamily(Device* device, uint32_t familyIndex, uint32_t queueCount);

			void AddSupportedQueueType(QueueType queueType);

			void Destroy() override;

			[[nodiscard]] const vk::Queue&       GetVkQueue(uint32_t index = 0) const;
			[[nodiscard]] const vk::CommandPool& GetCommandPool() const;

			[[nodiscard]] vk::CommandBuffer& BeginOneshotCommands();

			void EndOneshotCommands(uint32_t index = 0);

			CommandBuffer AllocateCommandBuffer(QueueType type, vk::CommandBufferLevel commandBufferLevel = vk::CommandBufferLevel::ePrimary, bool singleInstance = false);


		private:
			std::vector<vk::Queue> _vkQueues;
			uint32_t               _index;
			uint32_t               _queueCount;
			vk::CommandPool        _commandPool;
			BitSet<uint8_t>        _queueTypes;

			vk::CommandBuffer _currentOneshotCommandBuffer = VK_NULL_HANDLE;

			void DeallocateCommandBuffer(CommandBuffer& commandBuffer) const;
	};
}
