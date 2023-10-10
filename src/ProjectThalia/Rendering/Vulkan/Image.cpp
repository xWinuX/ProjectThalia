#include "ProjectThalia/Rendering/Vulkan/Image.hpp"
#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Image::Image(const Device* device, const char* pixels, vk::DeviceSize pixelsSizeInBytes) :
		DeviceObject(device)
	{
		Buffer buffer = Buffer::CreateTransferBuffer(device, pixels, pixelsSizeInBytes);
		buffer.CopyData(pixels, pixelsSizeInBytes, 0);

		//vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo({}, VK_IMAGE_TYPE_2D, )
	}
}