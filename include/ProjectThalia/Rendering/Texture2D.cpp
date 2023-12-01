#include "Texture2D.hpp"
#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

#include <utility>

namespace ProjectThalia::Rendering
{
	Texture2D::Texture2D(std::string filePath, TextureSettings textureSettings):
		_textureSettings(textureSettings)
	{
		_imageFile = IO::ImageFile(std::move(filePath), IO::ImageFile::ChannelSetup::RGBA);
		_image = Vulkan::Image(Vulkan::Context::GetDevice(), _imageFile.GetPixels())
	}

	Texture2D::~Texture2D()
	{

	}
}