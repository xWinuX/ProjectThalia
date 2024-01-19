#pragma once

#include "Buffer.hpp"
#include "DeviceObject.hpp"

#include "InFlightResource.hpp"
#include "SplitEngine/DataStructures.hpp"
#include "vulkan/vulkan.hpp"

#include <stack>
#include <unordered_set>
#include <vector>

namespace SplitEngine::Rendering::Vulkan
{
	class Device;

	class Buffer;

	class DescriptorSetAllocator final : DeviceObject
	{
		public:
			struct BufferCreateInfo
			{
				public:
					bool SingleInstance = false;
					bool DeviceLocal    = false;
					bool Cached         = false;
			};

			struct CreateInfo
			{
				public:
					std::unordered_set<uint32_t>                     bindings                 = std::unordered_set<uint32_t>();
					std::vector<vk::DescriptorSetLayoutBinding>      descriptorLayoutBindings = std::vector<vk::DescriptorSetLayoutBinding>(0);
					std::vector<vk::DescriptorPoolSize>              descriptorPoolSizes      = std::vector<vk::DescriptorPoolSize>(0);
					std::vector<std::vector<vk::WriteDescriptorSet>> writeDescriptorSets      = std::vector<std::vector<vk::WriteDescriptorSet>>(0);
					std::vector<BufferCreateInfo>                    bufferCreateInfos        = std::vector<BufferCreateInfo>(0);
			};

			struct DescriptorPoolInstance
			{
				public:
					vk::DescriptorPool             DescriptorPool;
					std::vector<vk::DescriptorSet> DescriptorSets;
					AvailableStack<uint32_t>       Available;
			};

			struct DescriptorPoolAllocation
			{
					uint32_t DescriptorPoolIndex;
					uint32_t DescriptorSetIndex;
			};

			struct Allocation
			{
					friend DescriptorSetAllocator;

				public:
					InFlightResource<vk::DescriptorSet>                   DescriptorSets {};
					std::vector<Buffer>                                   ShaderBuffers            = std::vector<Buffer>(0);
					std::vector<InFlightResource<std::byte*>>             ShaderBufferPtrs         = std::vector<InFlightResource<std::byte*>>(0);
					std::vector<std::vector<vk::DescriptorImageInfo>>     ImageInfos               = std::vector<std::vector<vk::DescriptorImageInfo>>(0);
					std::vector<InFlightResource<vk::WriteDescriptorSet>> ImageWriteDescriptorSets = std::vector<InFlightResource<vk::WriteDescriptorSet>>(0);
					std::vector<uint32_t>                                 SparseShaderBufferLookup = std::vector<uint32_t>(12);
					std::vector<uint32_t>                                 SparseImageLookup        = std::vector<uint32_t>(12);

				private:
					std::vector<DescriptorPoolAllocation> _descriptorPoolAllocations;
			};

		public:
			DescriptorSetAllocator() = default;
			DescriptorSetAllocator(Device* device, CreateInfo& descriptorSetInfo, uint32_t maxSetsPerPool);

			void Destroy() override;

			Allocation AllocateDescriptorSet();

			void DeallocateDescriptorSet(Allocation& descriptorSetAllocation);

			[[nodiscard]] const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

		private:
			std::vector<uint32_t>                            _bindings;
			vk::DescriptorSetLayout                          _descriptorSetLayout;
			std::vector<vk::DescriptorSetLayout>             _descriptorSetLayouts;
			std::vector<DescriptorPoolInstance>              _descriptorPoolInstances;
			std::vector<vk::DescriptorPoolSize>              _descriptorPoolSizes;
			std::vector<std::vector<vk::WriteDescriptorSet>> _writeDescriptorSets;
			std::vector<BufferCreateInfo>                    _bufferCreateInfo;
			uint32_t                                         _maxSetsPerPool = 10;

			void AllocateNewDescriptorPool();
	};
}