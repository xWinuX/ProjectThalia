#include "SplitEngine/Rendering/Texture2D.hpp"

#include "SplitEngine/Rendering/Vulkan/Instance.hpp"

namespace SplitEngine::Rendering
{
	Texture2D::Texture2D(const CreateInfo& createInfo) :
		_ioImage(createInfo.IoImage),
		_vulkanImage(Vulkan::Image(&Vulkan::Instance::Get().GetPhysicalDevice().GetDevice(),
		                           _ioImage.Pixels.data(),
		                           _ioImage.Width * _ioImage.Height * _ioImage.Channels,
		                           { _ioImage.Width, _ioImage.Height, 1 },
		                           { .Format = GetVulkanFormat(_ioImage) })),
		_sampler(Vulkan::Instance::Get().GetAllocator().AllocateSampler(createInfo.TextureSettings)),
		_textureSettings(createInfo.TextureSettings) {}

	Texture2D::~Texture2D() { _vulkanImage.Destroy(); }

	const Vulkan::Image& Texture2D::GetImage() const { return _vulkanImage; }

	const vk::Sampler* Texture2D::GetSampler() const { return _sampler; }

	vk::Format Texture2D::GetVulkanFormat(const IO::Image& image)
	{
		vk::Format format{};
		switch (image.Channels)
		{
			case 1:
				format = vk::Format::eR8Unorm;
				break;
			case 2:
				format = vk::Format::eR8G8Unorm;
				break;
			case 4:
				format = vk::Format::eR8G8B8A8Srgb;
				break;
			default:
				format = vk::Format::eR8G8B8A8Srgb;
		}

		return format;
	}
}
