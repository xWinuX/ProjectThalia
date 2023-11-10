#pragma once

#include "DeviceObject.hpp"
#include "Instance.hpp"

#include <unordered_map>
#include <vk_mem_alloc.h>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	/**
	 * I would want to make this a sub struct of the allocator class, but theres a clang and gcc bug that
	 * causes a compile error when you specify default values for member variables in sub structs and use them as an parameter with a default value.
	 * So it has to be like this even if it's ugly af
	 * https://github.com/llvm/llvm-project/issues/36032
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96645
	 */
	struct AllocatorCreateInfo
	{
		public:
			uint32_t                            MaxDescriptorSetsPerPool = 1000;
			std::vector<vk::DescriptorPoolSize> DescriptorPoolSizes      = {{vk::DescriptorType::eSampler, 1000},
																			{vk::DescriptorType::eCombinedImageSampler, 1000},
																			{vk::DescriptorType::eSampledImage, 1000},
																			{vk::DescriptorType::eStorageImage, 1000},
																			{vk::DescriptorType::eUniformTexelBuffer, 1000},
																			{vk::DescriptorType::eStorageTexelBuffer, 1000},
																			{vk::DescriptorType::eUniformBuffer, 1000},
																			{vk::DescriptorType::eStorageBuffer, 1000},
																			{vk::DescriptorType::eUniformBufferDynamic, 1000},
																			{vk::DescriptorType::eStorageBufferDynamic, 1000},
																			{vk::DescriptorType::eInputAttachment, 1000}};
	};

	class Allocator : DeviceObject
	{
		public:
			enum MemoryAllocationCreateFlagBits : uint32_t
			{
				None          = 0,
				PersistentMap = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
			};

			enum MemoryUsage
			{
				CpuToGpu = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
				GpuOnly  = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
				CpuOnly  = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY,
			};

			struct MemoryAllocationInfo
			{
				public:
					void* MappedData = nullptr;
			};

			struct MemoryAllocation
			{
					friend Allocator;

				public:
					MemoryAllocationInfo AllocationInfo;

				private:
					VmaAllocation VmaAllocation = nullptr;
			};

			struct BufferAllocation : MemoryAllocation
			{
				public:
					vk::Buffer Buffer = VK_NULL_HANDLE;
			};

			struct ImageAllocation : MemoryAllocation
			{
				public:
					vk::Image Image = VK_NULL_HANDLE;
			};

			struct MemoryAllocationCreateInfo
			{
				public:
					MemoryUsage                               Usage;
					vk::Flags<MemoryAllocationCreateFlagBits> Flags = MemoryAllocationCreateFlagBits::None;
			};


		public:
			Allocator() = default;
			Allocator(Device* device, const Instance& instance, const AllocatorCreateInfo& createInfo = {});


			void Destroy() override;

			[[nodiscard]] BufferAllocation CreateBuffer(const vk::BufferCreateInfo&       bufferCreateInfo,
														const MemoryAllocationCreateInfo& memoryAllocationCreateInfo);

			[[nodiscard]] ImageAllocation CreateImage(const vk::ImageCreateInfo& imageCreateInfo, const MemoryAllocationCreateInfo& memoryAllocationCreateInfo);


			void DestroyBuffer(const BufferAllocation& bufferAllocation);

			void DestroyImage(const ImageAllocation& imageAllocation);

			void* MapMemory(const MemoryAllocation& memoryAllocation);

			void UnmapMemory(const MemoryAllocation& memoryAllocation);

		private:
			VmaAllocator _vmaAllocator = nullptr;


			std::vector<vk::DescriptorPool>     _descriptorPools;
			std::vector<vk::DescriptorPoolSize> _descriptorPoolConfiguration;
			std::vector<std::unordered_map<vk::DescriptorType, uint32_t>> _descriptorPool

					static VmaAllocationCreateInfo
					CreateVmaAllocationCreateInfo(const MemoryAllocationCreateInfo& memoryAllocationCreateInfo);
	};
}
