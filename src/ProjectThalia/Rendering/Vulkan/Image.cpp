#include "ProjectThalia/Rendering/Vulkan/Image.hpp"
#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/Rendering/Vulkan/Device.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Image::Image(const Device* device, const char* pixels, vk::DeviceSize pixelsSizeInBytes, vk::Extent3D extend) :
		DeviceObject(device)
	{
		Buffer buffer = Buffer::CreateTransferBuffer(device, pixels, pixelsSizeInBytes);
		buffer.CopyData(pixels, pixelsSizeInBytes, 0);

		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo({},
																  vk::ImageType::e2D,
																  _format,
																  extend,
																  1,
																  1,
																  vk::SampleCountFlagBits::e1,
																  vk::ImageTiling::eOptimal,
																  vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

		_vkImage = GetDevice()->GetVkDevice().createImage(imageCreateInfo);

		vk::MemoryRequirements memoryRequirements = device->GetVkDevice().getImageMemoryRequirements(_vkImage);


		int                    memoryTypeIndex    = GetDevice()->FindMemoryTypeIndex(memoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::MemoryAllocateInfo memoryAllocateInfo = vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);

		_memory = GetDevice()->GetVkDevice().allocateMemory(memoryAllocateInfo);

		// Copy buffer to image
		GetDevice()->GetVkDevice().bindImageMemory(_vkImage, _memory, 0);

		vk::CommandBuffer commandBuffer = GetDevice()->BeginOneshotCommands();

		TransitionLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);

		vk::ImageSubresourceLayers imageSubresourceLayers = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
		vk::BufferImageCopy        bufferImageCopy        = vk::BufferImageCopy(0, 0, 0, imageSubresourceLayers, {0, 0, 0}, extend);

		commandBuffer.copyBufferToImage(buffer.GetVkBuffer(), _vkImage, _layout, bufferImageCopy);

		TransitionLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);

		GetDevice()->EndOneshotCommands(commandBuffer);
	}

	void Image::TransitionLayout(vk::ImageLayout newLayout)
	{
		vk::CommandBuffer commandBuffer = GetDevice()->BeginOneshotCommands();

		TransitionLayout(commandBuffer, newLayout);

		GetDevice()->EndOneshotCommands(commandBuffer);
	}

	void Image::TransitionLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout newLayout)
	{
		vk::ImageSubresourceRange subresourceRange   = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
		vk::ImageMemoryBarrier    imageMemoryBarrier = vk::ImageMemoryBarrier({},
																			  {},
                                                                           _layout,
                                                                           newLayout,
                                                                           vk::QueueFamilyIgnored,
                                                                           vk::QueueFamilyIgnored,
                                                                           _vkImage,
                                                                           subresourceRange);
	}
}