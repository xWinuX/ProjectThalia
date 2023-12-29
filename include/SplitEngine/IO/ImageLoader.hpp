#pragma once

#include "Image.hpp"

#include <stb_image.h>
#include <string>
#include <vector>

namespace SplitEngine::IO
{

	class ImageLoader
	{
		public:
			ImageLoader() = delete;

			enum ChannelSetup
			{
				Mono      = STBI_grey,
				MonoAlpha = STBI_grey_alpha,
				RGB       = STBI_rgb,
				RGBA      = STBI_rgb_alpha
			};

			static Image Load(const std::string& filePath, ChannelSetup channelSetup = ChannelSetup::RGBA);
	};

}
