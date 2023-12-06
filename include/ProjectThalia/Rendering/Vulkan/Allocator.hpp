#pragma once

#include "DeviceObject.hpp"
#include "Instance.hpp"

#include <unordered_map>
#include <vk_mem_alloc.h>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Allocator : DeviceObject
	{
		public:
			Allocator() = default;
			Allocator(Device* device, const Instance& instance);

			void Destroy() override;

#pragma region Memory
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

			enum MemoryPropertyFlagBits : uint32_t
			{
				LocalDevice = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
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
					vk::Flags<MemoryAllocationCreateFlagBits> Flags         = MemoryAllocationCreateFlagBits::None;
					vk::Flags<MemoryPropertyFlagBits>         RequiredFlags = {};
			};

			[[nodiscard]] BufferAllocation CreateBuffer(const vk::BufferCreateInfo&       bufferCreateInfo,
														const MemoryAllocationCreateInfo& memoryAllocationCreateInfo);

			[[nodiscard]] ImageAllocation CreateImage(const vk::ImageCreateInfo& imageCreateInfo, const MemoryAllocationCreateInfo& memoryAllocationCreateInfo);


			void DestroyBuffer(const BufferAllocation& bufferAllocation);

			void DestroyImage(const ImageAllocation& imageAllocation);

			void* MapMemory(const MemoryAllocation& memoryAllocation);

			void UnmapMemory(const MemoryAllocation& memoryAllocation);

		private:
			VmaAllocator _vmaAllocator = nullptr;

			size_t _buffersAllocated = 0;
			size_t _imagesAllocated  = 0;

			static VmaAllocationCreateInfo CreateVmaAllocationCreateInfo(const MemoryAllocationCreateInfo& memoryAllocationCreateInfo);
#pragma endregion
	};
}
