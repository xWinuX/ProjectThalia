#pragma once

#include <string>
#include <vector>
#include "ImageSlicer.hpp"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

namespace SplitEngine::Tools
{
	class ImagePacker
	{
		public:
			ImagePacker() = default;

			struct PackingInfo
			{
				public:
					uint32_t PageIndex = 0;

					glm::uvec2 Size{};

					glm::vec2 UVTopLeft{};
					glm::vec2 UVTopRight{};
					glm::vec2 UVBottomLeft{};
					glm::vec2 UVBottomRight{};
			};

			struct PackingData
			{
				public:
					std::vector<IO::Image>   PageImages;
					std::vector<PackingInfo> PackingInfos;

					/**
					 * Maps the ids returned from the AddImage* functions to packing info
					 * For the AddImage function the vector will just contain a singular entry that contains an index that points to the packing info of the image
					 * For the AddRelatedImages the vector will contain multiple indices which each point to the related packing info
					 */
					std::vector<std::vector<uint64_t>> PackMapping;
			};

		public:
			uint64_t AddImage(const std::string& imagePath);
			uint64_t AddImage(IO::Image&& image);

			uint64_t AddRelatedImages(std::vector<IO::Image>& images);
			uint64_t AddRelatedImages(SplitEngine::Tools::ImageSlicer::SliceData& sliceData);
			uint64_t AddRelatedImages(SplitEngine::Tools::ImageSlicer::SliceData&& sliceData);


			PackingData Pack(uint32_t pageSize);

		private:
			struct ImageEntry
			{
				public:
					uint64_t  ID = 0;
					IO::Image Image;
			};

			uint64_t _id = 0;

			std::vector<std::string> _imagePaths;
			std::vector<ImageEntry>  _images;
	};
}
