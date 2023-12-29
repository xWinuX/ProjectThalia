#include "SplitEngine/Tools/ImageSlicer.hpp"
#include <format>

#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/IO/ImageLoader.hpp"

#include "SplitEngine/Debug/Log.hpp"

namespace SplitEngine::Tools
{
	ImageSlicer::SliceData ImageSlicer::Slice(const std::string& filePath, ImageSlicer::SliceOptions sliceOptions)
	{
		SliceData sliceData;

		IO::Image image = IO::ImageLoader::Load(filePath);

		uint32_t width  = sliceOptions.Size.x == 0 ? image.Width : sliceOptions.Size.x;
		uint32_t height = sliceOptions.Size.y == 0 ? image.Height : sliceOptions.Size.y;

		if (sliceOptions.Size.x == 0 && sliceOptions.Size.y == 0)
		{
			if (sliceOptions.NumImages == 0)
			{
				ErrorHandler::ThrowRuntimeError(std::format("slicing image {0} failed! slice options size and num image can't both be zero!", filePath));
			}

			width  = image.Width / sliceOptions.NumImages;
			height = image.Height;
		}

		uint32_t xCount = image.Width / width;
		uint32_t yCount = image.Height / height;

		if (xCount * yCount < sliceOptions.NumImages)
		{
			ErrorHandler::ThrowRuntimeError(
					std::format("slicing image {0} failed! {1} image cannot fit with given slicing parameters ", filePath, sliceOptions.NumImages));
		}

		bool earlyExit = false;
		for (uint32_t y = 0; y < yCount; ++y)
		{
			for (uint32_t x = 0; x < xCount; ++x)
			{
				sliceData.Images.emplace_back(std::vector<std::byte>(width * height * 4, {}), width, height, 4);

				uint32_t index = (y * xCount) + x;

				size_t gridOffsetX = x * width * 4;
				size_t gridOffsetY = y * height * image.Width * 4;

				for (uint32_t line = 0; line < height; line++)
				{
					size_t lineOffset      = (line * image.Width * 4);
					size_t sliceLineOffset = (line * width * 4);

					memcpy(sliceData.Images[index].Pixels.data() + sliceLineOffset, image.Pixels.data() + gridOffsetX + gridOffsetY + lineOffset, width * 4);
				}

				if (sliceOptions.NumImages != 0 && index + 1 == sliceOptions.NumImages)
				{
					earlyExit = true;
					break;
				}
			}

			if (earlyExit) { break; }
		}

		return sliceData;
	}
}
