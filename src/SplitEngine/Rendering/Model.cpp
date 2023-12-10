#include "SplitEngine/Rendering/Model.hpp"

namespace SplitEngine::Rendering
{
	const Vulkan::Buffer& Model::GetModelBuffer() const { return _modelBuffer; }

	Model::~Model() { _modelBuffer.Destroy(); }
}
