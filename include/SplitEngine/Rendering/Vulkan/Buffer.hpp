#pragma once

#include "Allocator.hpp"
#include "DeviceObject.hpp"
#include "SplitEngine/Debug/Log.hpp"

#include <vulkan/vulkan.hpp>

namespace SplitEngine::Rendering::Vulkan
{
	class Device;

	class Buffer final : public DeviceObject
	{
		private:
			struct SubBuffer
			{
				vk::DeviceSize SizeInBytes;
				size_t         NumElements;
				vk::DeviceSize ElementSizeInBytes;
				vk::DeviceSize OffsetInBytes;
			};

		public:
			Buffer() = default;

			Buffer(Device*                               device,
			       vk::Flags<vk::BufferUsageFlagBits>    usage,
			       vk::SharingMode                       sharingMode,
			       Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
			       const char**                          data,
			       const Buffer&                         buffer);

			Buffer(Device*                               device,
			       vk::Flags<vk::BufferUsageFlagBits>    usage,
			       vk::SharingMode                       sharingMode,
			       Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
			       uint32_t                              numSubBuffers,
			       vk::DeviceSize                        subBufferSizeInBytes);

			Buffer(Device*                               device,
			       vk::Flags<vk::BufferUsageFlagBits>    usage,
			       vk::SharingMode                       sharingMode,
			       Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
			       vk::DeviceSize                        bufferSizeInBytes,
			       vk::DeviceSize                        numSubBuffers           = 0,
			       const char**                          data                    = Buffer::EMPTY_DATA,
			       const vk::DeviceSize*                 bufferSizesInBytes      = nullptr,
			       const vk::DeviceSize*                 dataSizesInBytes        = nullptr,
			       const vk::DeviceSize*                 dataElementSizesInBytes = nullptr);

			Buffer(Device*                               device,
			       vk::Flags<vk::BufferUsageFlagBits>    usage,
			       vk::SharingMode                       sharingMode,
			       Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
			       vk::DeviceSize                        numSubBuffers,
			       const char**                          data,
			       const vk::DeviceSize*                 bufferSizesInBytes,
			       const vk::DeviceSize*                 dataSizesInBytes,
			       const vk::DeviceSize*                 dataElementSizesInBytes);

			Buffer(Device*                               device,
			       vk::Flags<vk::BufferUsageFlagBits>    usage,
			       vk::SharingMode                       sharingMode,
			       Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
			       vk::DeviceSize                        bufferSizeInBytes,
			       const char*                           data                   = nullptr,
			       vk::DeviceSize                        dataSizeInBytes        = 0,
			       vk::DeviceSize                        dataElementSizeInBytes = 0);

			Buffer(Device*                               device,
			       vk::Flags<vk::BufferUsageFlagBits>    usage,
			       vk::SharingMode                       sharingMode,
			       Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
			       const char*                           data,
			       vk::DeviceSize                        bufferSizeInBytes,
			       vk::DeviceSize                        dataElementSizeInBytes);

			void Destroy() override;

			void Copy(const Buffer& destinationBuffer) const;

			void Copy(const Buffer& destinationBuffer, vk::DeviceSize sourceOffsetInBytes, vk::DeviceSize destinationOffsetInBytes, vk::DeviceSize sizeInBytes) const;

			[[nodiscard]] void* Map() const;

			[[nodiscard]] void* GetMappedData() const;

			void Unmap() const;

			void Stage(const char** data) const;

			void Invalidate() const;

			void Flush() const;

			[[nodiscard]] const vk::Buffer& GetVkBuffer() const;

			[[nodiscard]] size_t GetNumSubBuffers() const;

			[[nodiscard]] size_t GetBufferElementNum(size_t index = 0) const;

			[[nodiscard]] size_t GetDataElementNum(size_t index = 0) const;

			[[nodiscard]] vk::DeviceSize GetSizeInBytes(size_t index = 0) const;

			template<typename T>
			void CopyData(const T* data, const vk::DeviceSize dataSizeInBytes, const uint32_t subBufferIndex = 0)
			{
				char* mappedData = GetMappedData() == nullptr ? Map<char>() : static_cast<char*>(GetMappedData());
				memcpy(mappedData + _subBuffers[subBufferIndex].OffsetInBytes, data, dataSizeInBytes);
				//Unmap();
			}

			template<typename T>
			T* Map() { return static_cast<T*>(Map()); }

			template<typename T>
			T* GetMappedData() const { return static_cast<T*>(GetMappedData()); }

			static const char* EMPTY_DATA[];

		private:
			Allocator::BufferAllocation _bufferAllocation{};
			vk::DeviceSize              _bufferSize = 0;

			std::vector<SubBuffer>                _subBuffers;
			Allocator::MemoryAllocationCreateInfo _allocationCreateInfo;

			void InitializeSubBuffers(unsigned long long int numSubBuffers,
			                          const vk::DeviceSize*  bufferSizesInBytes,
			                          const vk::DeviceSize*  dataSizesInBytes,
			                          const vk::DeviceSize*  dataElementSizesInBytes);

			void CreateBuffer(vk::Flags<vk::BufferUsageFlagBits>    usage,
			                  vk::SharingMode                       sharingMode,
			                  Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
			                  const char* const*                    data);
	};
}
