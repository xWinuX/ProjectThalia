#include "Sprite.hpp"

namespace SplitEngine::Rendering
{
	Sprite::Sprite(Sprite::CreateInfo createInfo) { _textureIDs = std::vector<uint64_t>(createInfo.PackingData.PackMapping[createInfo.PackerID]); }

	size_t Sprite::GetNumSubSprites() const { return _textureIDs.size(); }

	uint32_t Sprite::GetTextureID(uint32_t index) { return static_cast<uint32_t>(_textureIDs[index]); }
}
