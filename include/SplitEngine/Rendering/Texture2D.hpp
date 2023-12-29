#pragma once

#include "SplitEngine/IO/ImageLoader.hpp"
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
					IO::Image       IoImage;
					TextureSettings TextureSettings = {};
			};

		public:
			explicit Texture2D(const CreateInfo& createInfo);
			~Texture2D();

			[[nodiscard]] const Vulkan::Image& GetImage() const;
			[[nodiscard]] const vk::Sampler*   GetSampler() const;

		private:
			IO::Image             _ioImage;
			Vulkan::Image         _vulkanImage;
			const vk::Sampler*    _sampler {};
			const TextureSettings _textureSettings;
	};
}
