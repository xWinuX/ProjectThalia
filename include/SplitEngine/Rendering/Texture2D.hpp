#pragma once

#include "SplitEngine/IO/ImageFile.hpp"
#include "SplitEngine/Rendering/Vulkan/Image.hpp"
#include "TextureSettings.hpp"
#include <string>

namespace SplitEngine::Rendering
{

	class Texture2D
	{
		public:
			struct CreateInfo
			{
					std::string     FileName;
					TextureSettings TextureSettings = {};
			};

		public:
			explicit Texture2D(const CreateInfo& createInfo);
			~Texture2D();

			[[nodiscard]] const Vulkan::Image& GetImage() const;
			[[nodiscard]] const vk::Sampler*   GetSampler() const;

		private:
			IO::ImageFile         _imageFile;
			Vulkan::Image         _image;
			const vk::Sampler*    _sampler {};
			const TextureSettings _textureSettings;
	};
}
