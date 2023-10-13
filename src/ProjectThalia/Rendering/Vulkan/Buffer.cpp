#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Device.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
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

	Buffer::Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   vk::DeviceSize                        bufferSizeInBytes,
				   vk::DeviceSize                        numSubBuffers,
				   const char**                          data,
				   const vk::DeviceSize*                 bufferSizesInBytes,
				   const vk::DeviceSize*                 dataSizesInBytes,
				   const vk::DeviceSize*                 dataElementSizesInBytes) :
		DeviceObject(device)
	{
		InitializeSubBuffers(numSubBuffers, bufferSizesInBytes, dataSizesInBytes, dataElementSizesInBytes);

		CreateBuffer(usage, sharingMode, memoryPropertyFlags, data);
	}

	Buffer::Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   const char**                          data,
				   const Buffer&                         buffer) :
		_bufferSize(buffer._bufferSize),
		DeviceObject(device)
	{
		// Copy sub buffers
		_subBuffers = buffer._subBuffers;

		CreateBuffer(usage, sharingMode, memoryPropertyFlags, data);
	}

	void Buffer::CreateBuffer(vk::Flags<vk::BufferUsageFlagBits>&          usage,
							  vk::SharingMode&                             sharingMode,
							  const vk::Flags<vk::MemoryPropertyFlagBits>& memoryPropertyFlags,
							  const char* const*                           data)
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo({}, _bufferSize, usage, sharingMode);

		_vkBuffer = GetDevice()->GetVkDevice().createBuffer(bufferCreateInfo);

		vk::MemoryRequirements memoryRequirements;
		GetDevice()->GetVkDevice().getBufferMemoryRequirements(_vkBuffer, &memoryRequirements);

		// Find memory type
		int memoryType = GetDevice()->FindMemoryTypeIndex(memoryRequirements, memoryPropertyFlags);

		vk::MemoryAllocateInfo memoryAllocateInfo = vk::MemoryAllocateInfo(memoryRequirements.size, memoryType);
		_memory                                   = GetDevice()->GetVkDevice().allocateMemory(memoryAllocateInfo);

		GetDevice()->GetVkDevice().bindBufferMemory(_vkBuffer, _memory, 0);

		// Copy data
		if (data != nullptr)
		{
			void*  mappedData;
			size_t offset       = 0;
			bool   mappedBuffer = false;
			for (int i = 0; i < _subBuffers.size(); i++)
			{
				if (data[i] != nullptr)
				{
					if (!mappedBuffer)
					{
						mappedData   = Map(0, _bufferSize);
						mappedBuffer = true;
					}
					memcpy((char*) mappedData + offset, data[i], _subBuffers[i].sizeInBytes);
				}
				offset += _subBuffers[i].sizeInBytes;
			}

			if (mappedBuffer) { GetDevice()->GetVkDevice().unmapMemory(_memory); }
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

	Buffer::Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   vk::DeviceSize                        numSubBuffers,
				   const char**                          data,
				   const vk::DeviceSize*                 bufferSizesInBytes,
				   const size_t*                         dataSizesInBytes,
				   const vk::DeviceSize*                 dataElementSizesInBytes) :
		Buffer(device, usage, sharingMode, memoryPropertyFlags, 0, numSubBuffers, data, bufferSizesInBytes, dataSizesInBytes, dataElementSizesInBytes)
	{}

	Buffer::Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   vk::DeviceSize                        bufferSizeInBytes,
				   const char*                           data,
				   vk::DeviceSize                        dataSizeInBytes,
				   vk::DeviceSize                        dataElementSizeInBytes) :
		Buffer(device, usage, sharingMode, memoryPropertyFlags, bufferSizeInBytes, 1, &data, &dataSizeInBytes, &bufferSizeInBytes, &dataElementSizeInBytes)
	{}

	Buffer::Buffer(const Device*                         device,
				   vk::Flags<vk::BufferUsageFlagBits>    usage,
				   vk::SharingMode                       sharingMode,
				   vk::Flags<vk::MemoryPropertyFlagBits> memoryPropertyFlags,
				   const char*                           data,
				   vk::DeviceSize                        bufferSizeInBytes,
				   vk::DeviceSize                        dataElementSizeInBytes) :
		Buffer(device, usage, sharingMode, memoryPropertyFlags, bufferSizeInBytes, 1, &data, &bufferSizeInBytes, &bufferSizeInBytes, &dataElementSizeInBytes)
	{}

	const vk::Buffer& Buffer::GetVkBuffer() const { return _vkBuffer; }

	void Buffer::Copy(const Buffer& destinationBuffer)
	{
		vk::CommandBuffer commandBuffer = GetDevice()->BeginOneshotCommands();

		vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, _bufferSize);
		commandBuffer.copyBuffer(_vkBuffer, destinationBuffer.GetVkBuffer(), 1, &copyRegion);

		GetDevice()->EndOneshotCommands(commandBuffer);
	}

	void Buffer::Destroy()
	{
		Utility::DeleteDeviceHandle(GetDevice(), _vkBuffer);
		GetDevice()->GetVkDevice().freeMemory(_memory);
	}

	size_t Buffer::GetDataElementNum(size_t index) const { return _subBuffers[index].numElements; }

	size_t Buffer::GetNumSubBuffers() const { return _subBuffers.size(); }

	size_t Buffer::GetBufferElementNum(size_t index) const { return _subBuffers[index].numElements; }

	vk::DeviceSize Buffer::GetSizeInBytes(size_t index) const { return _subBuffers[0].sizeInBytes; }

	void* Buffer::Map(vk::DeviceSize offset, vk::DeviceSize size)
	{
		return GetDevice()->GetVkDevice().mapMemory(_memory, offset, size == 0 ? _bufferSize : size);
	}

	void Buffer::Unmap() { GetDevice()->GetVkDevice().unmapMemory(_memory); }

	void Buffer::Stage(const char** data)
	{
		Buffer stagingBuffer = Buffer(GetDevice(),
									  vk::BufferUsageFlagBits::eTransferSrc,
									  vk::SharingMode::eExclusive,
									  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
									  data,
									  *this);

		stagingBuffer.Copy(*this);
		stagingBuffer.Destroy();
	}
}
