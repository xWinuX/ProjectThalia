#pragma once

#include "SplitEngine/IO/Image.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace SplitEngine::Tools
{
	class ImageSlicer
	{
		public:
			ImageSlicer() = delete;

			struct SliceData
			{
				public:
					std::vector<IO::Image> Images {};
			};

			struct SliceOptions
			{
				public:
					// Number of sub textures inside the texture (leaving this value at 0 will cause the slicer to extract as many images as can fit
					uint32_t NumTextures = 0;

					/**
					 * Size of each individual sub texture.
					 * Leaving one of the elements on 0 assumes all textures are laid out on this axis (x = 0 -> laid out horizontally y = 0 -> laid out vertically)
					 * Leaving both on 0 requires NumTextures to be set, assumes the textures are laid out horizontally and consume the full height
					 */
					glm::uvec2 Size {0, 0};
			};

			static SliceData Slice(const std::string& filePath, SliceOptions sliceOptions);
	};
}
