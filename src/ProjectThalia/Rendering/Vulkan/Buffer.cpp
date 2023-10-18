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

	Buffer::Buffer(const Device*         device,
				   CreateInfo            createInfo,
				   vk::DeviceSize        bufferSizeInBytes,
				   vk::DeviceSize        numSubBuffers,
				   const char**          data,
				   const vk::DeviceSize* bufferSizesInBytes,
				   const vk::DeviceSize* dataSizesInBytes,
				   const vk::DeviceSize* dataElementSizesInBytes) :
		DeviceObject(device)
	{
		InitializeSubBuffers(numSubBuffers, bufferSizesInBytes, dataSizesInBytes, dataElementSizesInBytes);

		CreateBuffer(createInfo, data);
	}

	Buffer::Buffer(const Device* device, CreateInfo createInfo, const char** data, const Buffer& buffer) :
		_bufferSize(buffer._bufferSize),
		DeviceObject(device)
	{
		// Copy sub buffers
		_subBuffers = buffer._subBuffers;

		CreateBuffer(createInfo, data);
	}

	void Buffer::CreateBuffer(CreateInfo createInfo, const char* const* data)
	{
		const vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo({}, _bufferSize, createInfo.Usage, createInfo.SharingMode);

		VmaAllocationCreateInfo allocationCreateInfo = VmaAllocationCreateInfo();
		allocationCreateInfo.usage                   = createInfo.MemoryUsage;
		allocationCreateInfo.flags                   = createInfo.AllocationCreateFlags;
		allocationCreateInfo.priority                = 1.0f;

		VmaAllocationInfo allocationInfo;

		vmaCreateBuffer(GetDevice()->GetAllocator(),
						reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo),
						&allocationCreateInfo,
						reinterpret_cast<VkBuffer*>(&_vkBuffer),
						&_allocation,
						&allocationInfo);

		if (createInfo.AllocationCreateFlags == VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT) { _mappedData = allocationInfo.pMappedData; }

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
					memcpy(mappedData + offsetInBytes, data[i], _subBuffers[i].sizeInBytes);
				}
				offsetInBytes += _subBuffers[i].sizeInBytes;
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

	Buffer::Buffer(const Device*         device,
				   CreateInfo            createInfo,
				   vk::DeviceSize        numSubBuffers,
				   const char**          data,
				   const vk::DeviceSize* bufferSizesInBytes,
				   const size_t*         dataSizesInBytes,
				   const vk::DeviceSize* dataElementSizesInBytes) :
		Buffer(device, createInfo, 0, numSubBuffers, data, bufferSizesInBytes, dataSizesInBytes, dataElementSizesInBytes)
	{}

	Buffer::Buffer(const Device*  device,
				   CreateInfo     createInfo,
				   vk::DeviceSize bufferSizeInBytes,
				   const char*    data,
				   vk::DeviceSize dataSizeInBytes,
				   vk::DeviceSize dataElementSizeInBytes) :
		Buffer(device, createInfo, bufferSizeInBytes, 1, &data, &dataSizeInBytes, &bufferSizeInBytes, &dataElementSizeInBytes)
	{}

	Buffer::Buffer(const Device* device, CreateInfo createInfo, const char* data, vk::DeviceSize bufferSizeInBytes, vk::DeviceSize dataElementSizeInBytes) :
		Buffer(device, createInfo, bufferSizeInBytes, 1, &data, &bufferSizeInBytes, &bufferSizeInBytes, &dataElementSizeInBytes)
	{}

	const vk::Buffer& Buffer::GetVkBuffer() const { return _vkBuffer; }

	void Buffer::Copy(const Buffer& destinationBuffer)
	{
		vk::CommandBuffer commandBuffer = GetDevice()->BeginOneshotCommands();

		vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, _bufferSize);
		commandBuffer.copyBuffer(_vkBuffer, destinationBuffer.GetVkBuffer(), 1, &copyRegion);

		GetDevice()->EndOneshotCommands(commandBuffer);
	}

	void Buffer::Destroy() { vmaDestroyBuffer(GetDevice()->GetAllocator(), _vkBuffer, _allocation); }

	size_t Buffer::GetDataElementNum(size_t index) const { return _subBuffers[index].numElements; }

	size_t Buffer::GetNumSubBuffers() const { return _subBuffers.size(); }

	size_t Buffer::GetBufferElementNum(size_t index) const { return _subBuffers[index].numElements; }

	vk::DeviceSize Buffer::GetSizeInBytes(size_t index) const { return _subBuffers[0].sizeInBytes; }

	void* Buffer::Map()
	{
		void* mappedData;
		vmaMapMemory(GetDevice()->GetAllocator(), _allocation, &mappedData);
		return mappedData;
	}

	void Buffer::Unmap() { vmaUnmapMemory(GetDevice()->GetAllocator(), _allocation); }

	void Buffer::Stage(const char** data)
	{
		Buffer stagingBuffer = Buffer(GetDevice(),
									  {
											  vk::BufferUsageFlagBits::eTransferSrc,
											  vk::SharingMode::eExclusive,
											  VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY,
									  },
									  data,
									  *this);

		stagingBuffer.Copy(*this);
		stagingBuffer.Destroy();
	}

	void* Buffer::GetMappedData() { return _mappedData; }
}
