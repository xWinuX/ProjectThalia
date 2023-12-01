#include "ProjectThalia/Rendering/Vulkan/Allocator.hpp"

#include "ProjectThalia/Rendering/Vulkan/Device.hpp"

namespace ProjectThalia::Rendering::Vulkan
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

		return imageAllocation;
	}

	void Allocator::DestroyBuffer(const Allocator::BufferAllocation& bufferAllocation)
	{
		vmaDestroyBuffer(_vmaAllocator, bufferAllocation.Buffer, bufferAllocation.VmaAllocation);
	}

	void Allocator::DestroyImage(const Allocator::ImageAllocation& imageAllocation)
	{
		vmaDestroyImage(_vmaAllocator, imageAllocation.Image, imageAllocation.VmaAllocation);
	}

	void* Allocator::MapMemory(const Allocator::MemoryAllocation& memoryAllocation)
	{
		void* mappedData;

		vmaMapMemory(_vmaAllocator, memoryAllocation.VmaAllocation, &mappedData);

		return mappedData;
	}

	void Allocator::UnmapMemory(const Allocator::MemoryAllocation& memoryAllocation) { vmaUnmapMemory(_vmaAllocator, memoryAllocation.VmaAllocation); }

	void Allocator::Destroy() { vmaDestroyAllocator(_vmaAllocator); }

	VmaAllocationCreateInfo Allocator::CreateVmaAllocationCreateInfo(const Allocator::MemoryAllocationCreateInfo& memoryAllocationCreateInfo)
	{
		VmaAllocationCreateInfo allocationCreateInfo = VmaAllocationCreateInfo();
		allocationCreateInfo.usage                   = static_cast<VmaMemoryUsage>(memoryAllocationCreateInfo.Usage);
		allocationCreateInfo.flags                   = static_cast<VmaAllocationCreateFlags>(memoryAllocationCreateInfo.Flags);
		allocationCreateInfo.priority                = 1.0f;
		return allocationCreateInfo;
	}

	Allocator::SamplerAllocation Allocator::AllocateSampler(TextureSettings textureSettings)
	{
		for (auto& [mapTextureSetting, sampler]: _samplers)
		{

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
	}
}
