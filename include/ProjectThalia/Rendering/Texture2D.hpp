#pragma once

#include "ProjectThalia/IO/ImageFile.hpp"
#include "ProjectThalia/Rendering/Vulkan/Image.hpp"
#include "TextureSettings.hpp"
#include <string>

namespace ProjectThalia::Rendering
{
	class Texture2D
	{
		public:
			explicit Texture2D(std::string filePath, const TextureSettings&& textureSettings);
			~Texture2D();

		private:
			IO::ImageFile          _imageFile;
			Vulkan::Image          _image;
			const TextureSettings& _textureSettings;
	};
}
