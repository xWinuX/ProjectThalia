#pragma once

#include "DeviceObject.hpp"
#include "ProjectThalia/Rendering/Vertex.hpp"
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Buffer final : DeviceObject
	{
		public:
			Buffer() = default;

			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   vk::DeviceSize                        bufferSize,
				   const char*                           data,
				   vk::DeviceSize                        dataSize,
				   vk::DeviceSize                        dataStride);

			template<typename T>
			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   vk::DeviceSize                        bufferSize,
				   const T*                              data,
				   vk::DeviceSize                        dataSize) :
				Buffer(device, usage, sharingMode, memoryPropertyFlags, bufferSize, reinterpret_cast<const char*>(data), dataSize, sizeof(T)) {};

			template<typename T>
			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   const T*                              data,
				   vk::DeviceSize                        dataSize) :
				Buffer(device, usage, sharingMode, memoryPropertyFlags, dataSize, reinterpret_cast<const char*>(data), dataSize, sizeof(T)) {};

			template<typename T>
			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   const std::vector<T>&                 data) :
				Buffer(device,
					   usage,
					   sharingMode,
					   memoryPropertyFlags,
					   data.size() * sizeof(T),
					   reinterpret_cast<const char*>(data.data()),
					   data.size() * sizeof(T),
					   sizeof(T)) {};

			template<typename T>
			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   const std::vector<T>&                 data,
				   vk::DeviceSize                        bufferSize) :
				Buffer(device,
					   usage,
					   sharingMode,
					   memoryPropertyFlags,
					   bufferSize,
					   reinterpret_cast<const char*>(data.data()),
					   data.size() * sizeof(T),
					   sizeof(T)) {};

			template<typename T, int TSize>
			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   const std::array<T, TSize>&           data) :
				Buffer(device,
					   usage,
					   sharingMode,
					   memoryPropertyFlags,
					   TSize * sizeof(T),
					   reinterpret_cast<const char*>(data.data()),
					   TSize * sizeof(T),
					   sizeof(T)) {};

			template<typename T, int TSize>
			Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   const std::array<T, TSize>&           data,
				   vk::DeviceSize                        bufferSize) :
				Buffer(device,
					   usage,
					   sharingMode,
					   memoryPropertyFlags,
					   bufferSize,
					   reinterpret_cast<const char*>(data.data()),
					   TSize * sizeof(T),
					   sizeof(T)) {};

			[[nodiscard]] const vk::Buffer& GetVkBuffer() const;

			//void MapData();

			void Destroy() override;

			void Copy(const Buffer& destinationBuffer);

			template<typename T>
			static Buffer CreateStagingBuffer(const Device* device, const T* data, vk::DeviceSize dataSize)
			{
				return Buffer(device,
							  vk::BufferUsageFlagBits::eTransferSrc,
							  vk::SharingMode::eExclusive,
							  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							  data,
							  dataSize);
			}

			template<typename T>
			static Buffer CreateStagingBuffer(const Device* device, const std::vector<T>& data)
			{
				return CreateStagingBuffer(device, data.data(), data.size() * sizeof(T));
			}

			template<typename T>
			static Buffer CreateStagedVertexBuffer(const Device* device, const T* data, vk::DeviceSize dataSize)
			{
				Buffer vertexBuffer = Buffer(device,
											 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
											 vk::SharingMode::eExclusive,
											 vk::MemoryPropertyFlagBits::eDeviceLocal,
											 static_cast<const T*>(nullptr),
											 dataSize);

				Buffer stagingBuffer = CreateStagingBuffer(device, data, dataSize);

				stagingBuffer.Copy(vertexBuffer);

				stagingBuffer.Destroy();

				return vertexBuffer;
			}

			template<typename T>
			static Buffer CreateStagedVertexBuffer(const Device* device, const std::vector<T>& data)
			{
				return CreateStagedVertexBuffer(device, data.data(), data.size() * sizeof(T));
			}

		private:
			vk::Buffer       _vkBuffer;
			vk::DeviceMemory _memory;

			vk::DeviceSize _bufferSize = 0;
			vk::DeviceSize _dataSize   = 0;
			vk::DeviceSize _dataStride = 0;
	};
}