#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Buffer::Buffer(const vk::Device&                         device,
				   const vk::PhysicalDeviceMemoryProperties& memoryProperties,
				   const char*                               data,
				   vk::DeviceSize                            size,
				   vk::BufferUsageFlagBits                   usage,
				   vk::SharingMode                           sharingMode)
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo({}, size, usage, sharingMode);

		_vkBuffer = device.createBuffer(bufferCreateInfo);

		vk::MemoryRequirements memoryRequirements;
		device.getBufferMemoryRequirements(_vkBuffer, &memoryRequirements);

		// Find memory type
		int                     memoryType = -1;
		vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		for (int i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				memoryType = i;
				break;
			}
		}
		if (memoryType == -1) { ErrorHandler::ThrowRuntimeError("Failed to find suitable memory type!"); }

		vk::MemoryAllocateInfo memoryAllocateInfo = vk::MemoryAllocateInfo(memoryRequirements.size, memoryType);
		_memory                                   = device.allocateMemory(memoryAllocateInfo);

		device.bindBufferMemory(_vkBuffer, _memory, 0);

		if (data != nullptr)
		{
			void* mappedData = device.mapMemory(_memory, 0, size);
			memcpy(mappedData, data, size);
			device.unmapMemory(_memory);
		}
	}

	const vk::Buffer& Buffer::GetVkBuffer() const { return _vkBuffer; }

	void Buffer::Destroy(vk::Device device)
	{
		Utility::DeleteDeviceHandle(device, _vkBuffer);
		device.freeMemory(_memory);
	}
}
