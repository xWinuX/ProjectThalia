#include "ProjectThalia/Rendering/Material.hpp"
#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

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

	void Material::SetTexture(size_t index, const Texture2D& texture)
	{
		LOG("sdasd {0}", _descriptorSetAllocation.ImageWriteDescriptorSets.size());

		delete _descriptorSetAllocation.ImageWriteDescriptorSets[index].pImageInfo;
		_descriptorSetAllocation.ImageWriteDescriptorSets[index].pImageInfo = new vk::DescriptorImageInfo(*texture.GetSampler(),
																										  texture.GetImage().GetView(),
																										  texture.GetImage().GetLayout());

		_updateImageWriteDescriptorSets.push_back(_descriptorSetAllocation.ImageWriteDescriptorSets[index]);
	}

	void Material::Update()
	{
		if (!_updateImageWriteDescriptorSets.empty())
		{
			Vulkan::Context::GetDevice()->GetVkDevice().updateDescriptorSets(_updateImageWriteDescriptorSets, nullptr);
		}

		_updateImageWriteDescriptorSets.clear();
	}
}