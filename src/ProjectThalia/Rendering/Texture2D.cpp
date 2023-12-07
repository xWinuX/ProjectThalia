#include "ProjectThalia/Rendering/Texture2D.hpp"
#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

#include <utility>

namespace ProjectThalia::Rendering
{
	Texture2D::Texture2D(std::string filePath, const TextureSettings& textureSettings) :
		_textureSettings(textureSettings),
		_imageFile(IO::ImageFile(std::move(filePath), IO::ImageFile::ChannelSetup::RGBA)),
		_image(Vulkan::Image(Vulkan::Context::GetDevice(),
							 _imageFile.GetPixels(),
							 _imageFile.GetTotalImageSize(),
							 {static_cast<uint32_t>(_imageFile.GetWidth()), static_cast<uint32_t>(_imageFile.GetHeight()), 1},
							 {})),
		_sampler(Vulkan::Context::GetDevice()->GetAllocator().AllocateSampler(textureSettings))
	{}

	Texture2D::~Texture2D()
	{
		LOG("Texture destroy");
		_image.Destroy();
	}

	const Vulkan::Image& Texture2D::GetImage() const { return _image; }

	const vk::Sampler* Texture2D::GetSampler() const { return _sampler; }
}