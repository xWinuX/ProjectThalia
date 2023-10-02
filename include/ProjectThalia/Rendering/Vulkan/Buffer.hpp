#pragma once

#include "DeviceObject.hpp"
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
					vk::DeviceSize sizeInBytes;
					size_t         numElements;
					vk::DeviceSize elementSizeInBytes;
			};

		public:
			Buffer() = default;

			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   vk::DeviceSize                        numSubBuffers,
				   const char**                          data,
				   const vk::DeviceSize*                 bufferSizesInBytes,
				   const vk::DeviceSize*                 dataSize,
				   const vk::DeviceSize*                 dataElementSizesInBytes);

			void Destroy() override;

			void Copy(const Buffer& destinationBuffer);

			[[nodiscard]] const vk::Buffer& GetVkBuffer() const;
			[[nodiscard]] size_t            GetNumSubBuffers() const;
			[[nodiscard]] size_t            GetBufferElementNum(size_t index = 0) const;
			[[nodiscard]] size_t            GetDataElementNum(size_t index = 0) const;
			[[nodiscard]] vk::DeviceSize    GetSizeInBytes(size_t index = 0) const;

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
				const char*    data[]                    = {reinterpret_cast<const char*>(vertices), reinterpret_cast<const char*>(indices)};
				vk::DeviceSize dataSizesInBytes[]        = {numVertices * sizeof(TVertex), numIndices * sizeof(TIndex)};
				vk::DeviceSize dataElementSizesInBytes[] = {sizeof(TVertex), sizeof(TIndex)};

				Buffer stagingBuffer = Buffer(device,
											  vk::BufferUsageFlagBits::eTransferSrc,
											  vk::SharingMode::eExclusive,
											  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
											  2,
											  data,
											  dataSizesInBytes,
											  dataSizesInBytes,
											  dataElementSizesInBytes);

				Buffer modelBuffer = Buffer(device,
											vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer |
													vk::BufferUsageFlagBits::eIndexBuffer,
											vk::SharingMode::eExclusive,
											vk::MemoryPropertyFlagBits::eDeviceLocal,
											2,
											EMPTY_DATA,
											dataSizesInBytes,
											dataSizesInBytes,
											dataElementSizesInBytes);

				stagingBuffer.Copy(modelBuffer);
				stagingBuffer.Destroy();

				return modelBuffer;
			}

		private:
			static const char* EMPTY_DATA[];

			vk::DeviceSize   _bufferSize = 0;
			vk::Buffer       _vkBuffer;
			vk::DeviceMemory _memory;

			std::vector<SubBuffer> _subBuffers;
	};
}