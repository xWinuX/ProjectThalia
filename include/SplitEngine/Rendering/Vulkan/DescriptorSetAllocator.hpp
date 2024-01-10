#pragma once

#include "Buffer.hpp"
#include "DeviceObject.hpp"

#include "SplitEngine/DataStructures.hpp"
#include "vulkan/vulkan.hpp"

#include <set>
#include <stack>
#include <vector>

namespace SplitEngine::Rendering::Vulkan
{
	class Device;

	class Buffer;

	class DescriptorSetAllocator final : DeviceObject
	{
		public:
			struct CreateInfo
			{
				public:
					std::set<uint32_t>                          alreadyCoveredBindings   = std::set<uint32_t>();
					std::vector<vk::DescriptorSetLayoutBinding> descriptorLayoutBindings = std::vector<vk::DescriptorSetLayoutBinding>(0);
					std::vector<vk::DescriptorPoolSize>         descriptorPoolSizes      = std::vector<vk::DescriptorPoolSize>(0);
					std::vector<vk::WriteDescriptorSet>         writeDescriptorSets      = std::vector<vk::WriteDescriptorSet>(0);
			};

			struct DescriptorPoolInstance
			{
				public:
					vk::DescriptorPool             DescriptorPool;
					std::vector<vk::DescriptorSet> DescriptorSets;
					AvailableStack<uint32_t>       Available;
			};

			struct Allocation
			{
					friend DescriptorSetAllocator;

				public:
					vk::DescriptorSet                                 DescriptorSet            = VK_NULL_HANDLE;
					std::vector<Buffer>                               ShaderBuffers            = std::vector<Buffer>();
					std::vector<vk::WriteDescriptorSet>               ImageWriteDescriptorSets = std::vector<vk::WriteDescriptorSet>();
					std::vector<std::vector<vk::DescriptorImageInfo>> ImageInfos;

				private:
					uint32_t _descriptorPoolIndex = -1;
					uint32_t _descriptorSetIndex  = -1;
			};

		public:
			DescriptorSetAllocator() = default;
			DescriptorSetAllocator(Device* device, CreateInfo& descriptorSetInfo, uint32_t maxSetsPerPool);

			void Destroy() override;

			Allocation AllocateDescriptorSet();

			void DeallocateDescriptorSet(Allocation& descriptorSetAllocation);

			[[nodiscard]] const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

		private:
			vk::DescriptorSetLayout             _descriptorSetLayout;
			std::vector<DescriptorPoolInstance> _descriptorPoolInstances;
			std::vector<vk::DescriptorPoolSize> _descriptorPoolSizes;
			std::vector<vk::WriteDescriptorSet> _writeDescriptorSets;
			uint32_t                            _maxSetsPerPool = 10;

			void AllocateNewDescriptorPool();
	};
}