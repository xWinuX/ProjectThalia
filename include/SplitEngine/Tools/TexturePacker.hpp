#pragma once

#include "SplitEngine/IO/ImageFile.hpp"

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <string>
#include <vector>

namespace SplitEngine::Tools
{
	class TexturePacker
	{
		public:
			TexturePacker() = default;

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
					std::vector<std::vector<std::byte>> PageBytes;
					std::vector<TextureInfo> TextureInfos;
			};


		public:
			void AddTexture(const std::string& texturePath);

			PackingData Pack(uint32_t pageSize);

		private:
			std::vector<std::string> _texturePaths;
	};
}
