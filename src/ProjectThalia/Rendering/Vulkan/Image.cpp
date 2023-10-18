#include "ProjectThalia/Rendering/Vulkan/Image.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/Rendering/Vulkan/Device.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Image::Image(const Device* device, const char* pixels, vk::DeviceSize pixelsSizeInBytes, vk::Extent3D extend) :
		DeviceObject(device)
	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo({},
																  vk::ImageType::e2D,
																  _format,
																  extend,
																  1,
																  1,
																  vk::SampleCountFlagBits::e1,
																  vk::ImageTiling::eOptimal,
																  vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

		VmaAllocationCreateInfo allocationCreateInfo = VmaAllocationCreateInfo();
		allocationCreateInfo.usage                   = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
		allocationCreateInfo.flags                   = {};
		allocationCreateInfo.priority                = 1.0f;

		vmaCreateImage(GetDevice()->GetAllocator(),
					   reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo),
					   &allocationCreateInfo,
					   reinterpret_cast<VkImage*>(&_vkImage),
					   &_allocation,
					   nullptr);


		// Copy pixel data to image
		Buffer buffer = Buffer::CreateTransferBuffer(device, pixels, pixelsSizeInBytes);
		buffer.CopyData(pixels, pixelsSizeInBytes, 0);

		vk::CommandBuffer commandBuffer = GetDevice()->BeginOneshotCommands();

		TransitionLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);

		vk::ImageSubresourceLayers imageSubresourceLayers = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
		vk::BufferImageCopy        bufferImageCopy        = vk::BufferImageCopy(0, 0, 0, imageSubresourceLayers, {0, 0, 0}, extend);

		commandBuffer.copyBufferToImage(buffer.GetVkBuffer(), _vkImage, _layout, bufferImageCopy);

		TransitionLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);

		GetDevice()->EndOneshotCommands(commandBuffer);

		buffer.Destroy();

		// Create image view
		vk::ImageSubresourceRange imageSubresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
		vk::ImageViewCreateInfo   imageViewCreateInfo   = vk::ImageViewCreateInfo({}, _vkImage, vk::ImageViewType::e2D, _format, {}, imageSubresourceRange);

		_view = GetDevice()->GetVkDevice().createImageView(imageViewCreateInfo);
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


		vk::PipelineStageFlagBits sourceStage;
		vk::PipelineStageFlagBits destinationStage;

		if (_layout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (_layout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage      = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else { ErrorHandler::ThrowRuntimeError("Unsupported image layout transition"); }

		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);

		_layout = newLayout;
	}

	void Image::Destroy()
	{
		GetDevice()->GetVkDevice().destroyImageView(_view);
		vmaDestroyImage(GetDevice()->GetAllocator(), _vkImage, _allocation);
	}

	const vk::Image& Image::GetVkImage() const { return _vkImage; }

	const vk::ImageView& Image::GetView() const { return _view; }

	vk::Format Image::GetFormat() const { return _format; }

	vk::ImageLayout Image::GetLayout() const { return _layout; }
}