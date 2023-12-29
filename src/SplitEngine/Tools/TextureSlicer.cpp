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

		IO::Image imageFile = IO::ImageLoader::Load(filePath);

		uint32_t width  = sliceOptions.Size.x == 0 ? imageFile.Width : sliceOptions.Size.x;
		uint32_t height = sliceOptions.Size.y == 0 ? imageFile.Height : sliceOptions.Size.y;

		if (sliceOptions.Size.x == 0 && sliceOptions.Size.y == 0)
		{
			if (sliceOptions.NumTextures == 0)
			{
				ErrorHandler::ThrowRuntimeError(std::format("slicing texture {0} failed! slice options size and num textures can't both be zero!", filePath));
			}

			width  = imageFile.Width / sliceOptions.NumTextures;
			height = imageFile.Height;
		}

		uint32_t xCount = imageFile.Width / width;
		uint32_t yCount = imageFile.Height / height;

		if (xCount * yCount < sliceOptions.NumTextures)
		{
			ErrorHandler::ThrowRuntimeError(
					std::format("slicing texture {0} failed! {1} textures cannot fit with given slicing parameters ", filePath, sliceOptions.NumTextures));
		}

		bool earlyExit = false;
		for (uint32_t y = 0; y < yCount; ++y)
		{
			for (uint32_t x = 0; x < xCount; ++x)
			{
				sliceData.Images.emplace_back(std::vector<std::byte>(width * height * 4, {}), width, height, 4);

				uint32_t index = (y * xCount) + x;

				size_t gridOffsetX = x * width * 4;
				size_t gridOffsetY = y * height * imageFile.Width * 4;

				for (uint32_t line = 0; line < height; line++)
				{
					size_t lineOffset      = (line * imageFile.Width * 4);
					size_t sliceLineOffset = (line * width * 4);

					memcpy(sliceData.Images[index].Pixels.data() + sliceLineOffset,
						   imageFile.Pixels.data() + gridOffsetX + gridOffsetY + lineOffset,
						   width * 4);
				}

				if (sliceOptions.NumTextures != 0 && index + 1 == sliceOptions.NumTextures)
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
