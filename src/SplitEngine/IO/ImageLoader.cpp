#include "SplitEngine/IO/ImageLoader.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Debug/Log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace SplitEngine::IO
{
	Image ImageLoader::Load(const std::string& filePath, const ChannelSetup channelSetup)
	{
		int width, height;

		const stbi_uc* pixels = stbi_load(filePath.c_str(), &width, &height, nullptr, static_cast<int>(channelSetup));

		if (pixels == nullptr)
		{
			ErrorHandler::ThrowRuntimeError(std::format("failed to load image {0}! \n {1}", filePath, stbi_failure_reason()));
		}

		int numChannels = static_cast<int>(channelSetup);

		const size_t sizeInBytes = width * height * numChannels;

		Image image = Image(std::vector<std::byte>(sizeInBytes, {}), static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(numChannels));

		memcpy(image.Pixels.data(), pixels, sizeInBytes);

		delete pixels;

		return image;
	}
}
