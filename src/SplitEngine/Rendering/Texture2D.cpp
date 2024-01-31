#include "SplitEngine/Rendering/Texture2D.hpp"

#include "SplitEngine/Rendering/Vulkan/Context.hpp"

namespace SplitEngine::Rendering
{
	Texture2D::Texture2D(const CreateInfo& createInfo) :
		_ioImage(createInfo.IoImage),
		_vulkanImage(Vulkan::Image(Vulkan::Context::GetDevice(),
		                           _ioImage.Pixels.data(),
		                           _ioImage.Width * _ioImage.Height * _ioImage.Channels,
		                           { _ioImage.Width, _ioImage.Height, 1 },
		                           {})),
		_sampler(Vulkan::Context::GetDevice()->GetAllocator().AllocateSampler(createInfo.TextureSettings)),
		_textureSettings(createInfo.TextureSettings) {}

	Texture2D::~Texture2D() { _vulkanImage.Destroy(); }

	const Vulkan::Image& Texture2D::GetImage() const { return _vulkanImage; }

	const vk::Sampler* Texture2D::GetSampler() const { return _sampler; }
}
