#include "ProjectThalia/Rendering/Model.hpp"

namespace ProjectThalia::Rendering
{
	const Vulkan::Buffer& Model::GetModelBuffer() const { return _modelBuffer; }

	Model::~Model() { _modelBuffer.Destroy(); }
}
