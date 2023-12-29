#include "SplitEngine/Debug/Log.hpp"
#include "SplitEngine/Tools/ImagePacker.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#include "SplitEngine/ErrorHandler.hpp"

namespace SplitEngine::Tools
{
	void ImagePacker::AddImage(const std::string& imagePath) { _imagePaths.push_back(imagePath); }

	ImagePacker::PackingData ImagePacker::Pack(uint32_t pageSize)
	{
		PackingData packingData {};

		std::vector<std::vector<stbrp_rect>> atlasPageRects;
		std::vector<IO::Image>               images;
		std::vector<stbrp_rect>              rects = std::vector<stbrp_rect>();

		for (int i = 0; i < _imagePaths.size(); ++i)
		{
			IO::Image image = IO::ImageLoader::Load(_imagePaths[i], IO::ImageLoader::ChannelSetup::RGBA);

			if (image.Width > pageSize || image.Height > pageSize)
			{
				ErrorHandler::ThrowRuntimeError(std::format("Image {0} ({1}x{2}) is bigger than the specified page size ({3}x{3})!",
															_imagePaths[i],
															image.Width,
															image.Height,
															pageSize));
			}

			rects.push_back({i, static_cast<int32_t>(image.Width), static_cast<int32_t>(image.Height)});
			images.push_back(image);
		}

		// Pack rects
		while (!rects.empty())
		{
			stbrp_context context;
			stbrp_node*   nodes = new stbrp_node[1024];

			stbrp_init_target(&context, static_cast<int32_t>(pageSize), static_cast<int32_t>(pageSize), nodes, 1024);

			int32_t numPacked = stbrp_pack_rects(&context, rects.data(), static_cast<int32_t>(rects.size()));

			std::vector<stbrp_rect> pageRects = std::vector<stbrp_rect>();
			pageRects.reserve(numPacked);

			// Remove rects that where packed and add them to the page
			rects.erase(std::remove_if(rects.begin(),
									   rects.end(),
									   [&pageRects](stbrp_rect rect) {
										   if (rect.was_packed)
										   {
											   pageRects.push_back(rect);
											   return true;
										   }

										   return false;
									   }),
						rects.end());

			atlasPageRects.push_back(pageRects);

			delete[] nodes;
		}

		// Create page images
		float xStep = 1.0f / static_cast<float>(pageSize);
		float yStep = 1.0f / static_cast<float>(pageSize);
		for (uint32_t i = 0; i < atlasPageRects.size(); ++i)
		{
			packingData.PageImages.emplace_back(std::vector<std::byte>(pageSize * pageSize * 4, {}), pageSize, pageSize, 4);

			for (const stbrp_rect& rect : atlasPageRects[i])
			{
				IO::Image image = images[rect.id];

				// Configure UV's
				float startX = static_cast<float>(rect.x);
				float startY = static_cast<float>(rect.y);
				float xEnd   = startX + static_cast<float>(rect.w);
				float yEnd   = startY + static_cast<float>(rect.h);

				PackingInfo packingInfo {};
				packingInfo.PageIndex     = i;
				packingInfo.UVTopLeft     = {startX * xStep, startY * yStep};
				packingInfo.UVTopRight    = {xEnd * xStep, startY * yStep};
				packingInfo.UVBottomLeft  = {startX * xStep, yEnd * yStep};
				packingInfo.UVBottomRight = {xEnd * xStep, yEnd * yStep};

				packingData.PackingInfos.emplace_back(packingInfo);

				// Copy pixel data into image
				for (int y = 0; y < image.Height; ++y)
				{
					size_t xPageOffset = rect.x * 4;
					size_t yPageOffset = (rect.y * pageSize * 4) + (y * pageSize * 4);

					size_t yOffset = image.Width * y * 4;
					memcpy(packingData.PageImages[i].Pixels.data() + xPageOffset + yPageOffset, image.Pixels.data() + yOffset, image.Width * 4);
				}
			}
		}

		images.clear();
		_imagePaths.clear();

		return packingData;
	}
}
