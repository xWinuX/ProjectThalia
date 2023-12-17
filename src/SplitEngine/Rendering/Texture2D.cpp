#include "SplitEngine/Rendering/Texture2D.hpp"
#include "SplitEngine/Rendering/Vulkan/Context.hpp"

#include <utility>

namespace SplitEngine::Rendering
{
	Texture2D::Texture2D(const CreateInfo& createInfo) :
		_textureSettings(createInfo.TextureSettings),
		_imageFile(IO::ImageFile(createInfo.FileName, IO::ImageFile::ChannelSetup::RGBA)),
		_image(Vulkan::Image(Vulkan::Context::GetDevice(),
							 _imageFile.GetPixels(),
							 _imageFile.GetTotalImageSize(),
							 {static_cast<uint32_t>(_imageFile.GetWidth()), static_cast<uint32_t>(_imageFile.GetHeight()), 1},
							 {})),
		_sampler(Vulkan::Context::GetDevice()->GetAllocator().AllocateSampler(createInfo.TextureSettings))
	{}

	Texture2D::~Texture2D()
	{
		_image.Destroy();
	}

	const Vulkan::Image& Texture2D::GetImage() const { return _image; }

	const vk::Sampler* Texture2D::GetSampler() const { return _sampler; }
}