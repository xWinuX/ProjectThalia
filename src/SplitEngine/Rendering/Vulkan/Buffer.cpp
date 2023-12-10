#include "SplitEngine/Rendering/Vulkan/Buffer.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Rendering/Vulkan/Device.hpp"
#include "SplitEngine/Rendering/Vulkan/Utility.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	const char* Buffer::EMPTY_DATA[] = {
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
	};

	Buffer::Buffer(Device*                               device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
				   vk::DeviceSize                        bufferSizeInBytes,
				   vk::DeviceSize                        numSubBuffers,
				   const char**                          data,
				   const vk::DeviceSize*                 bufferSizesInBytes,
				   const vk::DeviceSize*                 dataSizesInBytes,
				   const vk::DeviceSize*                 dataElementSizesInBytes) :
		DeviceObject(device)
	{
		InitializeSubBuffers(numSubBuffers, bufferSizesInBytes, dataSizesInBytes, dataElementSizesInBytes);

		CreateBuffer(usage, sharingMode, allocationCreateInfo, data);
	}

	Buffer::Buffer(Device*                               device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
				   const char**                          data,
				   const Buffer&                         buffer) :
		_bufferSize(buffer._bufferSize),
		DeviceObject(device)
	{
		// Copy sub buffers
		_subBuffers = buffer._subBuffers;

		CreateBuffer(usage, sharingMode, allocationCreateInfo, data);
	}

	void Buffer::CreateBuffer(vk::Flags<vk::BufferUsageFlagBits>    usage,
							  vk::SharingMode                       sharingMode,
							  Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
							  const char* const*                    data)
	{
		const vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo({}, _bufferSize, usage, sharingMode);

		Allocator::MemoryAllocationInfo allocationInfo;

		_bufferAllocation = GetDevice()->GetAllocator().CreateBuffer(bufferCreateInfo, allocationCreateInfo);

		// Copy data
		if (data != nullptr)
		{
			char*  mappedData;
			size_t offsetInBytes = 0;
			bool   mappedBuffer  = false;
			for (int i = 0; i < _subBuffers.size(); i++)
			{
				if (data[i] != nullptr)
				{
					if (!mappedBuffer)
					{
						mappedData   = Map<char>();
						mappedBuffer = true;
					}
					memcpy(mappedData + offsetInBytes, data[i], _subBuffers[i].SizeInBytes);
				}
				offsetInBytes += _subBuffers[i].SizeInBytes;
			}

			if (mappedBuffer) { Unmap(); }
		}
	}

	void Buffer::InitializeSubBuffers(unsigned long long int numSubBuffers,
									  const vk::DeviceSize*  bufferSizesInBytes,
									  const vk::DeviceSize*  dataSizesInBytes,
									  const vk::DeviceSize*  dataElementSizesInBytes)
	{
		if (numSubBuffers > 12) { ErrorHandler::ThrowRuntimeError("Can't have more than 12 sub buffers!"); }

		bool bufferSizeFromSubBuffers = _bufferSize == 0;

		_subBuffers.resize(numSubBuffers);
		for (int i = 0; i < numSubBuffers; i++)
		{
			_subBuffers[i] = {bufferSizesInBytes[i], dataSizesInBytes[i] / dataElementSizesInBytes[i], dataElementSizesInBytes[i], _bufferSize};
			if (bufferSizeFromSubBuffers) { _bufferSize += bufferSizesInBytes[i]; }
		}
	}

	Buffer::Buffer(Device*                               device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
				   vk::DeviceSize                        numSubBuffers,
				   const char**                          data,
				   const vk::DeviceSize*                 bufferSizesInBytes,
				   const size_t*                         dataSizesInBytes,
				   const vk::DeviceSize*                 dataElementSizesInBytes) :
		Buffer(device, usage, sharingMode, allocationCreateInfo, 0, numSubBuffers, data, bufferSizesInBytes, dataSizesInBytes, dataElementSizesInBytes)
	{}

	Buffer::Buffer(Device*                               device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
				   vk::DeviceSize                        bufferSizeInBytes,
				   const char*                           data,
				   vk::DeviceSize                        dataSizeInBytes,
				   vk::DeviceSize                        dataElementSizeInBytes) :
		Buffer(device, usage, sharingMode, allocationCreateInfo, bufferSizeInBytes, 1, &data, &dataSizeInBytes, &bufferSizeInBytes, &dataElementSizeInBytes)
	{}

	Buffer::Buffer(Device*                               device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   Allocator::MemoryAllocationCreateInfo allocationCreateInfo,
				   const char*                           data,
				   vk::DeviceSize                        bufferSizeInBytes,
				   vk::DeviceSize                        dataElementSizeInBytes) :
		Buffer(device, usage, sharingMode, allocationCreateInfo, bufferSizeInBytes, 1, &data, &bufferSizeInBytes, &bufferSizeInBytes, &dataElementSizeInBytes)
	{}

	const vk::Buffer& Buffer::GetVkBuffer() const { return _bufferAllocation.Buffer; }

	void Buffer::Copy(const Buffer& destinationBuffer)
	{
		vk::CommandBuffer commandBuffer = GetDevice()->BeginOneshotCommands();

		vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, _bufferSize);
		commandBuffer.copyBuffer(GetVkBuffer(), destinationBuffer.GetVkBuffer(), 1, &copyRegion);

		GetDevice()->EndOneshotCommands(commandBuffer);
	}

	void Buffer::Destroy() { GetDevice()->GetAllocator().DestroyBuffer(_bufferAllocation); }

	size_t Buffer::GetDataElementNum(size_t index) const { return _subBuffers[index].NumElements; }

	size_t Buffer::GetNumSubBuffers() const { return _subBuffers.size(); }

	size_t Buffer::GetBufferElementNum(size_t index) const { return _subBuffers[index].NumElements; }

	vk::DeviceSize Buffer::GetSizeInBytes(size_t index) const { return _subBuffers[0].SizeInBytes; }

	void* Buffer::Map() { return GetDevice()->GetAllocator().MapMemory(_bufferAllocation); }

	void Buffer::Unmap() { GetDevice()->GetAllocator().UnmapMemory(_bufferAllocation); }

	void Buffer::Stage(const char** data)
	{
		Buffer stagingBuffer = Buffer(GetDevice(),
									  vk::BufferUsageFlagBits::eTransferSrc,
									  vk::SharingMode::eExclusive,
									  {
											  Allocator::CpuOnly,
									  },
									  data,
									  *this);

		stagingBuffer.Copy(*this);
		stagingBuffer.Destroy();
	}

	void* Buffer::GetMappedData() { return _bufferAllocation.AllocationInfo.MappedData; }

	void Buffer::Invalidate() { GetDevice()->GetAllocator().InvalidateBuffer(_bufferAllocation); }

	void Buffer::Flush() { GetDevice()->GetAllocator().FlushBuffer(_bufferAllocation); }
}
