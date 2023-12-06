#include "ProjectThalia/Rendering/Vulkan/Image.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/Rendering/Vulkan/Device.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Image::Image(Device* device, const unsigned char* pixels, vk::DeviceSize pixelsSizeInBytes, vk::Extent3D extend, CreateInfo createInfo) :
		DeviceObject(device)
	{
		vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo({},
																  vk::ImageType::e2D,
																  createInfo.Format,
																  extend,
																  1,
																  1,
																  vk::SampleCountFlagBits::e1,
																  vk::ImageTiling::eOptimal,
																  createInfo.Usage);

		Allocator::MemoryAllocationCreateInfo allocationCreateInfo {};
		allocationCreateInfo.Usage         = Allocator::GpuOnly;
		allocationCreateInfo.RequiredFlags = Allocator::LocalDevice;

		_imageAllocation = GetDevice()->GetAllocator().CreateImage(imageCreateInfo, allocationCreateInfo);

		vk::CommandBuffer commandBuffer;
		if (createInfo.TransitionLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal) { commandBuffer = GetDevice()->BeginOneshotCommands(); }

		// Copy pixel data to image
		Buffer transferBuffer;
		if (pixels)
		{
			transferBuffer = Buffer::CreateTransferBuffer(device, pixels, pixelsSizeInBytes);
			transferBuffer.CopyData(pixels, pixelsSizeInBytes, 0);

			TransitionLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);

			vk::ImageSubresourceLayers imageSubresourceLayers = vk::ImageSubresourceLayers(createInfo.AspectMask, 0, 0, 1);
			vk::BufferImageCopy        bufferImageCopy        = vk::BufferImageCopy(0, 0, 0, imageSubresourceLayers, {0, 0, 0}, extend);

			commandBuffer.copyBufferToImage(transferBuffer.GetVkBuffer(), GetVkImage(), _layout, bufferImageCopy);
		}


		if (createInfo.TransitionLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			TransitionLayout(commandBuffer, createInfo.TransitionLayout);


			GetDevice()->EndOneshotCommands(commandBuffer);
			transferBuffer.Destroy();
		}

		// Create image view
		vk::ImageSubresourceRange imageSubresourceRange = vk::ImageSubresourceRange(createInfo.AspectMask, 0, 1, 0, 1);
		vk::ImageViewCreateInfo   imageViewCreateInfo   = vk::ImageViewCreateInfo({},
                                                                              GetVkImage(),
                                                                              vk::ImageViewType::e2D,
                                                                              createInfo.Format,
																				  {},
                                                                              imageSubresourceRange);

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
		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) { return; }

		vk::ImageSubresourceRange subresourceRange   = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
		vk::ImageMemoryBarrier    imageMemoryBarrier = vk::ImageMemoryBarrier({},
																			  {},
                                                                           _layout,
                                                                           newLayout,
                                                                           vk::QueueFamilyIgnored,
                                                                           vk::QueueFamilyIgnored,
                                                                           GetVkImage(),
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
		else if (_layout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else { ErrorHandler::ThrowRuntimeError("Unsupported image layout transition"); }

		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);

		_layout = newLayout;
	}

	void Image::Destroy()
	{
		Utility::DeleteDeviceHandle(GetDevice(), _view);
		GetDevice()->GetAllocator().DestroyImage(_imageAllocation);
	}

	const vk::Image& Image::GetVkImage() const { return _imageAllocation.Image; }

	const vk::ImageView& Image::GetView() const { return _view; }

	vk::ImageLayout Image::GetLayout() const { return _layout; }
}