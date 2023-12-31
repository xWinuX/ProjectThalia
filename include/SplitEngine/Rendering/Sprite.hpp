#pragma once

#include "SplitEngine/Tools/ImagePacker.hpp"

#include <vector>

namespace SplitEngine::Rendering
{
	class Sprite
	{
		public:
			struct CreateInfo
			{
					uint64_t                         PackerID;
					Tools::ImagePacker::PackingData& PackingData;
			};

			explicit Sprite(CreateInfo createInfo);

			[[nodiscard]] size_t GetNumSubSprites() const;

			uint32_t GetTextureID(uint32_t index);

		private:
			std::vector<uint64_t> _textureIDs;
	};
}
