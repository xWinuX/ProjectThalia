#include "SplitEngine/Rendering/Material.hpp"

namespace SplitEngine::Rendering
{
	Material::Material(const CreateInfo& createInfo) :
		_shader(createInfo._shader),
		_instanceDescriptorSetAllocation(_shader->GetPipeline().AllocatePerInstanceDescriptorSet()),
		_instanceProperties(Shader::Properties(_shader.Get(), &_instanceDescriptorSetAllocation))
	{}

	AssetHandle<Shader> Material::GetShader() const { return _shader; }

	void Material::Update() { _instanceProperties.Update(); }

	Shader::Properties& Material::GetProperties() { return _instanceProperties; }

	void Material::Bind(vk::CommandBuffer& commandBuffer)
	{
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
										 _shader->GetPipeline().GetLayout(),
										 2,
										 1,
										 &_instanceDescriptorSetAllocation.DescriptorSets.Get(),
										 0,
										 nullptr);
	}

	Material::~Material() { _shader->GetPipeline().DeallocatePerInstanceDescriptorSet(_instanceDescriptorSetAllocation); }
}