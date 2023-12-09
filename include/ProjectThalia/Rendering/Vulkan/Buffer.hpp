#pragma once

#include "Allocator.hpp"
#include "DeviceObject.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/Rendering/Vertex.hpp"

#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Buffer final : DeviceObject
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

			void Copy(const Buffer& destinationBuffer);

			void* Map();

			void* GetMappedData();

			void Unmap();

			void Stage(const char** data);

			void Invalidate();

			void Flush();

			[[nodiscard]] const vk::Buffer& GetVkBuffer() const;
			[[nodiscard]] size_t            GetNumSubBuffers() const;
			[[nodiscard]] size_t            GetBufferElementNum(size_t index = 0) const;
			[[nodiscard]] size_t            GetDataElementNum(size_t index = 0) const;
			[[nodiscard]] vk::DeviceSize    GetSizeInBytes(size_t index = 0) const;

			template<typename T>
			void CopyData(const T* data, const vk::DeviceSize dataSizeInBytes, uint32_t subBufferIndex = 0)
			{
				char* mappedData = GetMappedData() == nullptr ? Map<char>() : static_cast<char*>(GetMappedData());
				memcpy(mappedData + _subBuffers[subBufferIndex].OffsetInBytes, data, dataSizeInBytes);
				//Unmap();
			}

			template<typename T>
			T* Map()
			{
				return static_cast<T*>(Map());
			}

			template<typename T>
			T* GetMappedData()
			{
				return static_cast<T*>(GetMappedData());
			}

			template<typename TVertex, typename TIndex>
			static Buffer CreateStagedModelBuffer(Device* device, const std::vector<TVertex>& vertices, const std::vector<TIndex>& indices)
			{
				return CreateStagedModelBuffer(device, vertices.data(), vertices.size(), indices.data(), indices.size());
			}

			template<typename TVertex, typename TIndex>
			static Buffer CreateStagedModelBuffer(Device*        device,
												  const TVertex* vertices,
												  vk::DeviceSize numVertices,
												  const TIndex*  indices,
												  vk::DeviceSize numIndices)
			{
				const char*    data[]                    = {reinterpret_cast<const char*>(vertices), reinterpret_cast<const char*>(indices)};
				vk::DeviceSize dataSizesInBytes[]        = {numVertices * sizeof(TVertex), numIndices * sizeof(TIndex)};
				vk::DeviceSize dataElementSizesInBytes[] = {sizeof(TVertex), sizeof(TIndex)};

				Buffer modelBuffer = Buffer(device,

											vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer |
													vk::BufferUsageFlagBits::eIndexBuffer,
											vk::SharingMode::eExclusive,
											{
													Allocator::GpuOnly,
											},
											2,
											EMPTY_DATA,
											dataSizesInBytes,
											dataSizesInBytes,
											dataElementSizesInBytes);

				modelBuffer.Stage(data);

				return modelBuffer;
			}

			template<typename T>
			static Buffer CreateUniformBuffer(Device* device, const T* data)
			{
				return Buffer(device,
							  vk::BufferUsageFlagBits::eUniformBuffer,
							  vk::SharingMode::eExclusive,
							  {Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap)},
							  reinterpret_cast<const char*>(data),
							  sizeof(T),
							  sizeof(T));
			}

			static Buffer CreateUniformBuffer(Device* device, size_t dataSizeInBytes, const char* data = nullptr)
			{
				return Buffer(device,
							  vk::BufferUsageFlagBits::eUniformBuffer,
							  vk::SharingMode::eExclusive,
							  {Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap)},
							  reinterpret_cast<const char*>(data),
							  dataSizeInBytes,
							  dataSizeInBytes);
			}

			static Buffer CreateStorageBuffer(Device* device, size_t dataSizeInBytes, const char* data = nullptr)
			{
				return Buffer(device,
							  vk::BufferUsageFlagBits::eStorageBuffer,
							  vk::SharingMode::eExclusive,
							  {Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::RandomAccess | Allocator::PersistentMap)},
							  reinterpret_cast<const char*>(data),
							  dataSizeInBytes,
							  dataSizeInBytes);
			}

			template<typename T>
			static Buffer CreateDynamicUniformBuffer(Device* device, const T* data)
			{
				return Buffer(device,
							  vk::BufferUsageFlagBits::eUniformBuffer,
							  vk::SharingMode::eExclusive,
							  {Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap)},
							  reinterpret_cast<const char*>(data),
							  sizeof(T),
							  sizeof(T));
			}

			template<typename T>
			static Buffer CreateStorageBuffer(Device* device, const T* data)
			{
				return Buffer(device,
							  vk::BufferUsageFlagBits::eStorageBuffer,
							  vk::SharingMode::eExclusive,
							  {Allocator::CpuToGpu,
							   vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::PersistentMap | Allocator::RandomAccess),
							   vk::Flags<Allocator::MemoryPropertyFlagBits>(Allocator::HostCached)},
							  reinterpret_cast<const char*>(data),
							  sizeof(T),
							  sizeof(T));
			}

			template<typename T>
			static Buffer CreateTransferBuffer(Device* device, const T* data, vk::DeviceSize dataSizeInBytes)
			{
				return Buffer(device,
							  vk::BufferUsageFlagBits::eTransferSrc,
							  vk::SharingMode::eExclusive,
							  {Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap)},
							  reinterpret_cast<const char*>(data),
							  dataSizeInBytes,
							  sizeof(T));
			}

		protected:
			static const char* EMPTY_DATA[];

		private:
			Allocator::BufferAllocation _bufferAllocation;
			vk::DeviceSize              _bufferSize = 0;

			std::vector<SubBuffer> _subBuffers;

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