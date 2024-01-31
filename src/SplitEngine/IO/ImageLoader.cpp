#include "SplitEngine/IO/ImageLoader.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Debug/Log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace SplitEngine::IO
{
	Image ImageLoader::Load(const std::string& filePath, const ImageLoader::ChannelSetup channelSetup)
	{
		int width, height, channels;

		const stbi_uc* pixels = stbi_load(filePath.c_str(), &width, &height, &channels, channelSetup);

		if (pixels == nullptr) { ErrorHandler::ThrowRuntimeError(std::format("failed to load image {0}!", filePath)); }

		int numChannels = 0;
		switch (channelSetup)
		{
			case ChannelSetup::RGB:
				numChannels = 3;
				break;
			case ChannelSetup::RGBA:
				numChannels = 4;
				break;
			case ChannelSetup::Mono:
				numChannels = 1;
				break;
			case ChannelSetup::MonoAlpha:
				numChannels = 2;
				break;
		}

		const size_t sizeInBytes = width * height * numChannels;

		Image image = Image(std::vector<std::byte>(sizeInBytes, {}), static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(channels));

		memcpy(image.Pixels.data(), pixels, sizeInBytes);

		delete pixels;

		return image;
	}
}
