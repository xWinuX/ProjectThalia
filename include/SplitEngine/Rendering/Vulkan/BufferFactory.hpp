#pragma once

#include <vulkan/vulkan.hpp>
#include "Allocator.hpp"
#include "Buffer.hpp"
#include "DeviceObject.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	class BufferFactory
	{
		public:
			template<typename TVertex, typename TIndex>
			static Buffer CreateStagedModelBuffer(Device* device, const std::vector<TVertex>& vertices, const std::vector<TIndex>& indices)
			{
				return CreateStagedModelBuffer(device, vertices.data(), vertices.size(), indices.data(), indices.size());
			}

			template<typename TVertex, typename TIndex>
			static Buffer CreateStagedModelBuffer(Device* device, const TVertex* vertices, const vk::DeviceSize numVertices, const TIndex* indices, const vk::DeviceSize numIndices)
			{
				const char*          data[]                    = { reinterpret_cast<const char*>(vertices), reinterpret_cast<const char*>(indices) };
				const vk::DeviceSize dataSizesInBytes[]        = { numVertices * sizeof(TVertex), numIndices * sizeof(TIndex) };
				const vk::DeviceSize dataElementSizesInBytes[] = { sizeof(TVertex), sizeof(TIndex) };

				Buffer modelBuffer = Buffer(device,
				                            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
				                            vk::SharingMode::eExclusive,
				                            { Allocator::GpuOnly, },
				                            2,
				                            Buffer::EMPTY_DATA,
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
				              { Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap) },
				              reinterpret_cast<const char*>(data),
				              sizeof(T),
				              sizeof(T));
			}

			static Buffer CreateUniformBuffer(Device* device, size_t dataSizeInBytes, const char* data = nullptr)
			{
				return Buffer(device,
				              vk::BufferUsageFlagBits::eUniformBuffer,
				              vk::SharingMode::eExclusive,
				              { Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap) },
				              data,
				              dataSizeInBytes,
				              dataSizeInBytes);
			}

			static Buffer CreateStorageBuffer(Device* device, size_t dataSizeInBytes, const char* data = nullptr)
			{
				return Buffer(device,
				              vk::BufferUsageFlagBits::eStorageBuffer,
				              vk::SharingMode::eExclusive,
				              { Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap) },
				              data,
				              dataSizeInBytes,
				              dataSizeInBytes);
			}

			template<typename T>
			static Buffer CreateDynamicUniformBuffer(Device* device, const T* data)
			{
				return Buffer(device,
				              vk::BufferUsageFlagBits::eUniformBuffer,
				              vk::SharingMode::eExclusive,
				              { Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap) },
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
				              {
					              Allocator::CpuToGpu,
					              vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::PersistentMap | Allocator::RandomAccess),
					              vk::Flags<Allocator::MemoryPropertyFlagBits>(Allocator::HostCached)
				              },
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
				              { Allocator::Auto, vk::Flags<Allocator::MemoryAllocationCreateFlagBits>(Allocator::WriteSequentially | Allocator::PersistentMap) },
				              reinterpret_cast<const char*>(data),
				              dataSizeInBytes,
				              sizeof(T));
			}
	};
}
