#include "SplitEngine/Rendering/Vulkan/Allocator.hpp"

#include "SplitEngine/Debug/Log.hpp"
#include "SplitEngine/Rendering/Vulkan/Device.hpp"
#include "SplitEngine/Rendering/Vulkan/Utility.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	Allocator::Allocator(Device* device, const Instance& instance) :
		DeviceObject(device)
	{
		// Create VMA Allocator
		VmaAllocatorCreateInfo vmaAllocatorCreateInfo = VmaAllocatorCreateInfo();
		vmaAllocatorCreateInfo.vulkanApiVersion       = VK_API_VERSION_1_3;
		vmaAllocatorCreateInfo.device                 = device->GetVkDevice();
		vmaAllocatorCreateInfo.physicalDevice         = device->GetPhysicalDevice().GetVkPhysicalDevice();
		vmaAllocatorCreateInfo.instance               = instance.GetVkInstance();

		vmaCreateAllocator(&vmaAllocatorCreateInfo, &_vmaAllocator);
	}

	Allocator::BufferAllocation Allocator::CreateBuffer(const vk::BufferCreateInfo&                  bufferCreateInfo,
														const Allocator::MemoryAllocationCreateInfo& memoryAllocationCreateInfo)
	{
		BufferAllocation bufferAllocation;

		VmaAllocationCreateInfo allocationCreateInfo = CreateVmaAllocationCreateInfo(memoryAllocationCreateInfo);

		VmaAllocationInfo allocationInfo;

		vmaCreateBuffer(_vmaAllocator,
						reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo),
						&allocationCreateInfo,
						reinterpret_cast<VkBuffer*>(&bufferAllocation.Buffer),
						&bufferAllocation.VmaAllocation,
						&allocationInfo);

		bufferAllocation.AllocationInfo.MappedData = allocationInfo.pMappedData;

		_buffersAllocated++;

		return bufferAllocation;
	}

	Allocator::ImageAllocation Allocator::CreateImage(const vk::ImageCreateInfo&                   imageCreateInfo,
													  const Allocator::MemoryAllocationCreateInfo& memoryAllocationCreateInfo)
	{
		ImageAllocation imageAllocation;

		VmaAllocationCreateInfo allocationCreateInfo = CreateVmaAllocationCreateInfo(memoryAllocationCreateInfo);

		vmaCreateImage(_vmaAllocator,
					   reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo),
					   &allocationCreateInfo,
					   reinterpret_cast<VkImage*>(&imageAllocation.Image),
					   &imageAllocation.VmaAllocation,
					   nullptr);

		_imagesAllocated++;

		return imageAllocation;
	}

	void Allocator::DestroyBuffer(const Allocator::BufferAllocation& bufferAllocation)
	{
		vmaDestroyBuffer(_vmaAllocator, bufferAllocation.Buffer, bufferAllocation.VmaAllocation);
		_buffersAllocated--;
	}

	void Allocator::DestroyImage(const Allocator::ImageAllocation& imageAllocation)
	{
		vmaDestroyImage(_vmaAllocator, imageAllocation.Image, imageAllocation.VmaAllocation);
		_imagesAllocated--;
	}

	void* Allocator::MapMemory(const Allocator::MemoryAllocation& memoryAllocation)
	{
		void* mappedData;

		vmaMapMemory(_vmaAllocator, memoryAllocation.VmaAllocation, &mappedData);

		return mappedData;
	}

	void Allocator::UnmapMemory(const Allocator::MemoryAllocation& memoryAllocation) { vmaUnmapMemory(_vmaAllocator, memoryAllocation.VmaAllocation); }

	void Allocator::Destroy()
	{
		LOG("Leaked buffers {0}", _buffersAllocated);
		LOG("Leaked images {0}", _imagesAllocated);
		vmaDestroyAllocator(_vmaAllocator);
		for (const SamplerEntry& samplerEntry : _samplers)
		{
			//Utility::DeleteDeviceHandle(GetDevice(), samplerEntry.Sampler);
		}
	}

	VmaAllocationCreateInfo Allocator::CreateVmaAllocationCreateInfo(const Allocator::MemoryAllocationCreateInfo& memoryAllocationCreateInfo)
	{
		VmaAllocationCreateInfo allocationCreateInfo = VmaAllocationCreateInfo();
		allocationCreateInfo.usage                   = static_cast<VmaMemoryUsage>(memoryAllocationCreateInfo.Usage);
		allocationCreateInfo.flags                   = static_cast<VmaAllocationCreateFlags>(memoryAllocationCreateInfo.Flags);
		allocationCreateInfo.requiredFlags           = static_cast<VkMemoryPropertyFlags>(memoryAllocationCreateInfo.RequiredFlags);
		allocationCreateInfo.priority                = 1.0f;
		return allocationCreateInfo;
	}

	const vk::Sampler* Allocator::AllocateSampler(TextureSettings textureSettings)
	{
		for (const SamplerEntry& samplerEntry : _samplers)
		{
			if (samplerEntry.TextureSettings == textureSettings) { return &samplerEntry.Sampler; }
		}

		vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo({},
																		static_cast<vk::Filter>(textureSettings.MagnificationFilter),
																		static_cast<vk::Filter>(textureSettings.MinificationFilter),
																		static_cast<vk::SamplerMipmapMode>(textureSettings.MipmapMode),
																		static_cast<vk::SamplerAddressMode>(textureSettings.WrapMode.x),
																		static_cast<vk::SamplerAddressMode>(textureSettings.WrapMode.y),
																		static_cast<vk::SamplerAddressMode>(textureSettings.WrapMode.z),
																		textureSettings.MipLodBias,
																		textureSettings.MaxAnisotropy > 0.0f,
																		textureSettings.MaxAnisotropy,
																		vk::False,
																		vk::CompareOp::eNever,
																		textureSettings.MinLod,
																		textureSettings.MaxLod,
																		vk::BorderColor::eIntOpaqueBlack,
																		vk::False);

		_samplers.push_back({textureSettings, GetDevice()->GetVkDevice().createSampler(samplerCreateInfo)});

		return &_samplers.back().Sampler;
	}

	void Allocator::InvalidateBuffer(const Allocator::BufferAllocation& bufferAllocation)
	{
		vmaInvalidateAllocation(_vmaAllocator, bufferAllocation.VmaAllocation, 0, VK_WHOLE_SIZE);
	}

	void Allocator::FlushBuffer(const Allocator::BufferAllocation& bufferAllocation)
	{
		vmaFlushAllocation(_vmaAllocator, bufferAllocation.VmaAllocation, 0, VK_WHOLE_SIZE);
	}
}