#include "ProjectThalia/Rendering/Material.hpp"

namespace ProjectThalia::Rendering
{
	Material::Material(Shader* shader) :
		_shader(shader)
	{
		_descriptorSetAllocation = shader->GetPipeline().GetDescriptorSetManager().AllocateDescriptorSet();
	}

	Material::~Material()
	{
		if (_descriptorSetAllocation.DescriptorSet != VK_NULL_HANDLE)
		{
			_shader->GetPipeline().GetDescriptorSetManager().DeallocateDescriptorSet(_descriptorSetAllocation);
		}
	}

	Shader* Material::GetShader() const { return _shader; }

	Vulkan::DescriptorSetManager::DescriptorSetAllocation& Material::GetDescriptorSetAllocation() { return _descriptorSetAllocation; }

	const Vulkan::DescriptorSetManager::DescriptorSetAllocation& Material::GetDescriptorSetAllocation() const { return _descriptorSetAllocation; }
}