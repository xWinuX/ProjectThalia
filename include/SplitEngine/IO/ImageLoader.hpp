#pragma once

#include <filesystem>

#include "Image.hpp"

#include <stb_image.h>
#include <string>

namespace SplitEngine::IO
{
	class ImageLoader
	{
		public:
			ImageLoader() = delete;

			enum class ChannelSetup
			{
				Mono      = 1,

				MonoAlpha = 2,

				RGBA = 4,

				/**
				 * Since most gpus don't support 24 bit images RGB images will still be 32 bit (RGBA) images
				 */
				RGB = RGBA,
			};

			static Image Load(const std::filesystem::path& filePath, ChannelSetup channelSetup = ChannelSetup::RGBA);
	};
}
