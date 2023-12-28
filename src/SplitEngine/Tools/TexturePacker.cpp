#include "SplitEngine/Tools/TexturePacker.hpp"
#include "SplitEngine/Debug/Log.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "SplitEngine/ErrorHandler.hpp"
#include "stb_image_write.h"
#include "stb_rect_pack.h"

namespace SplitEngine::Tools
{
	void TexturePacker::AddTexture(const std::string& texturePath) { _texturePaths.push_back(texturePath); }

	TexturePacker::PackingData TexturePacker::Pack(uint32_t pageSize)
	{
		PackingData packingData {};

		std::vector<std::vector<stbrp_rect>> atlasPageRects;
		std::vector<IO::ImageFile*>          imageFiles;
		std::vector<stbrp_rect>              rects = std::vector<stbrp_rect>();

		for (int i = 0; i < _texturePaths.size(); ++i)
		{
			IO::ImageFile* imageFile = new IO::ImageFile(_texturePaths[i], IO::ImageFile::ChannelSetup::RGBA);

			if (imageFile->GetWidth() > pageSize || imageFile->GetHeight() > pageSize)
			{
				ErrorHandler::ThrowRuntimeError(std::format("Texture {0} ({1}x{2}) is bigger than the specified page size ({3}x{3})!",
															_texturePaths[i],
															imageFile->GetWidth(),
															imageFile->GetHeight(),
															pageSize));
			}

			rects.push_back({i, imageFile->GetWidth(), imageFile->GetHeight()});
			imageFiles.push_back(imageFile);
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

		// Create Texture
		float xStep = 1.0f / static_cast<float>(pageSize);
		float yStep = 1.0f / static_cast<float>(pageSize);
		for (uint32_t i = 0; i < atlasPageRects.size(); ++i)
		{
			packingData.PageBytes.push_back(std::vector<std::byte>(pageSize * pageSize * 4, {}));

			for (const stbrp_rect& rect : atlasPageRects[i])
			{
				IO::ImageFile* imageFile = imageFiles[rect.id];

				// Configure UV's
				float startX = static_cast<float>(rect.x);
				float startY = static_cast<float>(rect.y);
				float xEnd   = startX + static_cast<float>(rect.w);
				float yEnd   = startY + static_cast<float>(rect.h);

				TextureInfo textureInfo {};
				textureInfo.PageIndex     = i;
				textureInfo.UVTopLeft     = {startX * xStep, startY * yStep};
				textureInfo.UVTopRight    = {xEnd * xStep, startY * yStep};
				textureInfo.UVBottomLeft  = {startX * xStep, yEnd * yStep};
				textureInfo.UVBottomRight = {xEnd * xStep, yEnd * yStep};

				packingData.TextureInfos.emplace_back(textureInfo);

				// Copy pixel data into texture
				for (int y = 0; y < imageFile->GetHeight(); ++y)
				{
					size_t xPageOffset = rect.x * 4;
					size_t yPageOffset = (rect.y * pageSize * 4) + (y * pageSize * 4);

					size_t yOffset = imageFile->GetWidth() * y * 4;
					memcpy(packingData.PageBytes[i].data() + xPageOffset + yPageOffset, imageFile->GetPixels() + yOffset, imageFile->GetWidth() * 4);
				}
			}

			stbi_write_png(std::format("TexturePage_{0}.png", i).c_str(),
						   static_cast<int32_t>(pageSize),
						   static_cast<int32_t>(pageSize),
						   4,
						   packingData.PageBytes[i].data(),
						   static_cast<int32_t>(pageSize) * 4);
		}

		// Delete resources
		for (const IO::ImageFile* imageFile : imageFiles) { delete imageFile; }

		imageFiles.clear();
		_texturePaths.clear();

		return packingData;
	}
}
