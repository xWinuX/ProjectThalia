#pragma once

#include "SplitEngine/IO/ImageLoader.hpp"

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <string>
#include <vector>

namespace SplitEngine::Tools
{
	class ImagePacker
	{
		public:
			ImagePacker() = default;

			struct TextureInfo
			{
				public:
					uint32_t  PageIndex = 0;
					glm::vec2 UVTopLeft {};
					glm::vec2 UVTopRight {};
					glm::vec2 UVBottomLeft {};
					glm::vec2 UVBottomRight {};
			};

			struct PackingData
			{
				public:
					std::vector<IO::Image>   PageImages;
					std::vector<TextureInfo> TextureInfos;
			};


		public:
			void AddImage(const std::string& texturePath);

			PackingData Pack(uint32_t pageSize);

		private:
			std::vector<std::string> _texturePaths;
	};
}
