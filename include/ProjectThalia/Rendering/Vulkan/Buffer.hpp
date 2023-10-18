#pragma once

#include "DeviceObject.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/Rendering/Vertex.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Buffer final : DeviceObject
	{
		public:
			struct CreateInfo
			{
					vk::Flags<vk::BufferUsageFlagBits> Usage;
					vk::SharingMode                    SharingMode;
					VmaMemoryUsage                     MemoryUsage;
					VmaAllocationCreateFlags           AllocationCreateFlags = {};
			};

		private:
			struct SubBuffer
			{
					vk::DeviceSize sizeInBytes;
					size_t         numElements;
					vk::DeviceSize elementSizeInBytes;
					vk::DeviceSize offsetInBytes;
			};

		public:
			Buffer() = default;
			Buffer(const Device* device, CreateInfo createInfo, const char** data, const Buffer& buffer);

			Buffer(const Device*         device,
				   CreateInfo            createInfo,
				   vk::DeviceSize        bufferSizeInBytes,
				   vk::DeviceSize        numSubBuffers           = 0,
				   const char**          data                    = EMPTY_DATA,
				   const vk::DeviceSize* bufferSizesInBytes      = nullptr,
				   const vk::DeviceSize* dataSizesInBytes        = nullptr,
				   const vk::DeviceSize* dataElementSizesInBytes = nullptr);

			Buffer(const Device*         device,
				   CreateInfo            createInfo,
				   vk::DeviceSize        numSubBuffers,
				   const char**          data,
				   const vk::DeviceSize* bufferSizesInBytes,
				   const vk::DeviceSize* dataSizesInBytes,
				   const vk::DeviceSize* dataElementSizesInBytes);

			Buffer(const Device*  device,
				   CreateInfo     createInfo,
				   vk::DeviceSize bufferSizeInBytes,
				   const char*    data                   = nullptr,
				   vk::DeviceSize dataSizeInBytes        = 0,
				   vk::DeviceSize dataElementSizeInBytes = 0);

			Buffer(const Device* device, CreateInfo createInfo, const char* data, vk::DeviceSize bufferSizeInBytes, vk::DeviceSize dataElementSizeInBytes);


			void Destroy() override;

			void Copy(const Buffer& destinationBuffer);

			void* Map();

			void* GetMappedData();

			void Unmap();

			void Stage(const char** data);

			[[nodiscard]] const vk::Buffer& GetVkBuffer() const;
			[[nodiscard]] size_t            GetNumSubBuffers() const;
			[[nodiscard]] size_t            GetBufferElementNum(size_t index = 0) const;
			[[nodiscard]] size_t            GetDataElementNum(size_t index = 0) const;
			[[nodiscard]] vk::DeviceSize    GetSizeInBytes(size_t index = 0) const;

			template<typename T>
			void CopyData(const T* data, const vk::DeviceSize dataSizeInBytes, uint32_t subBufferIndex = 0)
			{
				LOG("Mapping buffer data with offsetInBytes {0} and size {1}",
					_subBuffers[subBufferIndex].offsetInBytes,
					_subBuffers[subBufferIndex].sizeInBytes);
				char* mappedData = Map<char>();
				memcpy(mappedData + _subBuffers[subBufferIndex].offsetInBytes, data, dataSizeInBytes);
				Unmap();
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
			static Buffer CreateStagedModelBuffer(const Device* device, const std::vector<TVertex>& vertices, const std::vector<TIndex>& indices)
			{
				return CreateStagedModelBuffer(device, vertices.data(), vertices.size(), indices.data(), indices.size());
			}

			template<typename TVertex, typename TIndex>
			static Buffer CreateStagedModelBuffer(const Device*  device,
												  const TVertex* vertices,
												  vk::DeviceSize numVertices,
												  const TIndex*  indices,
												  vk::DeviceSize numIndices)
			{
				LOG("Create Staged Model Buffer");

				const char*    data[]                    = {reinterpret_cast<const char*>(vertices), reinterpret_cast<const char*>(indices)};
				vk::DeviceSize dataSizesInBytes[]        = {numVertices * sizeof(TVertex), numIndices * sizeof(TIndex)};
				vk::DeviceSize dataElementSizesInBytes[] = {sizeof(TVertex), sizeof(TIndex)};

				Buffer modelBuffer = Buffer(device,
											{
													vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer |
															vk::BufferUsageFlagBits::eIndexBuffer,
													vk::SharingMode::eExclusive,
													VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
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
			static Buffer CreateUniformBuffer(const Device* device, const T* data)
			{
				LOG("Create Uniform Buffer");
				return Buffer(device,
							  {vk::BufferUsageFlagBits::eUniformBuffer,
							   vk::SharingMode::eExclusive,
							   VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
							   VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT},
							  reinterpret_cast<const char*>(data),
							  sizeof(T),
							  sizeof(T));
			}

			template<typename T>
			static Buffer CreateStorageBuffer(const Device* device, const T* data)
			{
				LOG("Create Storage Buffer");
				return Buffer(device,
							  {
									  vk::BufferUsageFlagBits::eStorageBuffer,
									  vk::SharingMode::eExclusive,
									  VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
							  },
							  reinterpret_cast<const char*>(data),
							  sizeof(T),
							  sizeof(T));
			}

			template<typename T>
			static Buffer CreateTransferBuffer(const Device* device, const T* data, vk::DeviceSize dataSizeInBytes)
			{
				LOG("Create TransfserBuffer");
				return Buffer(device,
							  {
									  vk::BufferUsageFlagBits::eTransferSrc,
									  vk::SharingMode::eExclusive,
									  VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
							  },
							  reinterpret_cast<const char*>(data),
							  dataSizeInBytes,
							  sizeof(T));
			}

		private:
			static const char* EMPTY_DATA[];

			vk::Buffer     _vkBuffer;
			VmaAllocation  _allocation = nullptr;
			void*          _mappedData = nullptr;
			vk::DeviceSize _bufferSize = 0;

			std::vector<SubBuffer> _subBuffers;
			void                   InitializeSubBuffers(unsigned long long int numSubBuffers,
														const vk::DeviceSize*  bufferSizesInBytes,
														const vk::DeviceSize*  dataSizesInBytes,
														const vk::DeviceSize*  dataElementSizesInBytes);

			void CreateBuffer(CreateInfo createInfo, const char* const* data);
	};
}