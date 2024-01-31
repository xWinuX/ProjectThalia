#include "SplitEngine/Rendering/Model.hpp"

namespace SplitEngine::Rendering
{
	const Vulkan::Buffer& Model::GetModelBuffer() const { return _modelBuffer; }

	Model::~Model() { _modelBuffer.Destroy(); }

	void Model::Bind(const vk::CommandBuffer& commandBuffer) const
	{
		const vk::Buffer         vertexBuffers[] = { _modelBuffer.GetVkBuffer() };
		constexpr vk::DeviceSize offsets[]       = { 0 };

		commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(_modelBuffer.GetVkBuffer(), _modelBuffer.GetSizeInBytes(0), vk::IndexType::eUint16);
	}
}
