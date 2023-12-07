#pragma once

#include "Buffer.hpp"
#include "DeviceObject.hpp"

#include "ProjectThalia/DataStructures.hpp"
#include "vulkan/vulkan.hpp"

#include <stack>
#include <vector>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Buffer;

	class DescriptorSetManager final : DeviceObject
	{
		public:
			struct DescriptorPoolInstance
			{
				public:
					vk::DescriptorPool             DescriptorPool;
					std::vector<vk::DescriptorSet> DescriptorSets;
					AvailableStack<uint32_t>       Available;
			};

			struct DescriptorSetAllocation
			{
					friend DescriptorSetManager;

				public:
					vk::DescriptorSet                   DescriptorSet            = VK_NULL_HANDLE;
					std::vector<Buffer>                 ShaderBuffers            = std::vector<Buffer>();
					std::vector<vk::WriteDescriptorSet> ImageWriteDescriptorSets = std::vector<vk::WriteDescriptorSet>();

				private:
					uint32_t _descriptorPoolIndex = -1;
					uint32_t _descriptorSetIndex  = -1;
			};

		public:
			DescriptorSetManager() = default;
			DescriptorSetManager(Device*                                     device,
								 std::vector<vk::DescriptorSetLayoutBinding> descriptorLayoutBindings,
								 std::vector<vk::DescriptorPoolSize>         descriptorPoolSizes,
								 std::vector<vk::WriteDescriptorSet>         writeDescriptorSets,
								 uint32_t                                    maxSetsPerPool);

			void Destroy() override;

			DescriptorSetAllocation AllocateDescriptorSet();

			void DeallocateDescriptorSet(DescriptorSetAllocation& descriptorSetAllocation);

			[[nodiscard]] const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

		private:
			vk::DescriptorSetLayout             _descriptorSetLayout;
			std::vector<DescriptorPoolInstance> _descriptorPoolInstances;
			std::vector<vk::DescriptorPoolSize> _descriptorPoolSizes;
			std::vector<vk::WriteDescriptorSet> _writeDescriptorSets;
			uint32_t                            _maxSetsPerPool = 10;
			void                                AllocateNewDescriptorPool();
	};
}