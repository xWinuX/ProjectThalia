#include "SplitEngine/Tools/ImagePacker.hpp"
#include "SplitEngine/Debug/Log.hpp"
#include <algorithm>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Tools/ImageSlicer.hpp"

namespace SplitEngine::Tools
{
	uint64_t ImagePacker::AddImage(const std::string& imagePath)
	{
		_images.push_back({_id++, IO::ImageLoader::Load(imagePath)});
		return _images.back().ID;
	}

	uint64_t ImagePacker::AddImage(IO::Image&& image)
	{
		_images.push_back({_id++, std::move(image)});
		return _images.back().ID;
	}

	uint64_t ImagePacker::AddRelatedImages(ImageSlicer::SliceData& sliceData) { return AddRelatedImages(sliceData.Images); }

	uint64_t ImagePacker::AddRelatedImages(Tools::ImageSlicer::SliceData&& sliceData) { return AddRelatedImages(sliceData.Images); }

	uint64_t ImagePacker::AddRelatedImages(std::vector<IO::Image>& images)
	{
		uint64_t id = _id++;
		for (IO::Image& image : images) { _images.push_back({id, std::move(image)}); }
		images.clear();
		return id;
	}

	ImagePacker::PackingData ImagePacker::Pack(uint32_t pageSize)
	{
		PackingData packingData {};

		packingData.PackMapping = std::vector<std::vector<uint64_t>>(_id);

		std::vector<std::vector<stbrp_rect>> atlasPageRects;
		std::vector<stbrp_rect>              rects = std::vector<stbrp_rect>();

		// Check size and create rects
		for (int i = 0; i < _images.size(); ++i)
		{
			ImageEntry& imageEntry = _images[i];
			if (imageEntry.Image.Width > pageSize || imageEntry.Image.Height > pageSize)
			{
				ErrorHandler::ThrowRuntimeError(std::format("Image {0} ({1}x{2}) is bigger than the specified page size ({3}x{3})!",
															_imagePaths[i],
															imageEntry.Image.Width,
															imageEntry.Image.Height,
															pageSize));
			}

			rects.push_back({i, static_cast<int32_t>(imageEntry.Image.Width), static_cast<int32_t>(imageEntry.Image.Height)});
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
		std::vector<uint64_t> rectIDToPackingIndex = std::vector<uint64_t>(_images.size());

		float xStep = 1.0f / static_cast<float>(pageSize);
		float yStep = 1.0f / static_cast<float>(pageSize);
		for (uint32_t i = 0; i < atlasPageRects.size(); ++i)
		{
			packingData.PageImages.emplace_back(std::vector<std::byte>(pageSize * pageSize * 4, {}), pageSize, pageSize, 4);

			for (const stbrp_rect& rect : atlasPageRects[i])
			{
				ImageEntry imageEntry = _images[rect.id];

				// Configure UV's
				float startX = static_cast<float>(rect.x);
				float startY = static_cast<float>(rect.y);
				float xEnd   = startX + static_cast<float>(rect.w);
				float yEnd   = startY + static_cast<float>(rect.h);

				PackingInfo packingInfo {};
				packingInfo.PageIndex     = i;
				packingInfo.AspectRatio   = static_cast<float>(rect.w) / static_cast<float>(rect.h);
				packingInfo.UVTopLeft     = {startX * xStep, startY * yStep};
				packingInfo.UVTopRight    = {xEnd * xStep, startY * yStep};
				packingInfo.UVBottomLeft  = {startX * xStep, yEnd * yStep};
				packingInfo.UVBottomRight = {xEnd * xStep, yEnd * yStep};

				packingData.PackingInfos.emplace_back(packingInfo);

				// Create mapping from rect id to packing index
				rectIDToPackingIndex[rect.id] = packingData.PackingInfos.size() - 1;

				// Temporarily store rect id inside the packing mapping so we can sort it later
				packingData.PackMapping[imageEntry.ID].emplace_back(rect.id);

				// Copy pixel data into imageEntry
				for (int y = 0; y < imageEntry.Image.Height; ++y)
				{
					size_t xPageOffset = rect.x * 4;
					size_t yPageOffset = (rect.y * pageSize * 4) + (y * pageSize * 4);

					size_t yOffset = imageEntry.Image.Width * y * 4;
					memcpy(packingData.PageImages[i].Pixels.data() + xPageOffset + yPageOffset,
						   imageEntry.Image.Pixels.data() + yOffset,
						   imageEntry.Image.Width * 4);
				}
			}
		}

		// Dereference rect id to packing index
		for (std::vector<uint64_t>& mapping : packingData.PackMapping)
		{
			if (mapping.size() > 1) { std::sort(mapping.begin(), mapping.end()); }

			for (uint64_t& packingIndex : mapping) { packingIndex = rectIDToPackingIndex[packingIndex]; }
		}

		_images.clear();
		_imagePaths.clear();
		_id = -1;

		return packingData;
	}
}
