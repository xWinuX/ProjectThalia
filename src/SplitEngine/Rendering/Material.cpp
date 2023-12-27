#include "SplitEngine/Rendering/Material.hpp"
#include "SplitEngine/Rendering/Vulkan/Context.hpp"

namespace SplitEngine::Rendering
{
	Material::Material(const CreateInfo& createInfo) :
		_shader(createInfo._shader)
	{
		_descriptorSetAllocation = _shader->GetPipeline().GetDescriptorSetManager().AllocateDescriptorSet();
	}

	Material::~Material()
	{
		LOG("Mat destroy start");
		if (_descriptorSetAllocation.DescriptorSet != VK_NULL_HANDLE)
		{
			_shader->GetPipeline().GetDescriptorSetManager().DeallocateDescriptorSet(_descriptorSetAllocation);
		}
		LOG("Mat destroy end");
	}

	AssetHandle<Shader> Material::GetShader() const { return _shader; }

	Vulkan::DescriptorSetManager::DescriptorSetAllocation& Material::GetDescriptorSetAllocation() { return _descriptorSetAllocation; }

	const Vulkan::DescriptorSetManager::DescriptorSetAllocation& Material::GetDescriptorSetAllocation() const { return _descriptorSetAllocation; }

	void Material::SetTexture(size_t index, const Texture2D& texture)
	{
		_descriptorSetAllocation.ImageInfos[index][0] = vk::DescriptorImageInfo(*texture.GetSampler(),
																				texture.GetImage().GetView(),
																				texture.GetImage().GetLayout());
		SetWriteDescriptorSetDirty(index);
	}

	void Material::SetTextures(size_t index, size_t offset, std::vector<AssetHandle<Texture2D>>& textures)
	{
		for (int i = 0; i < textures.size(); ++i)
		{
			_descriptorSetAllocation.ImageInfos[index][i + offset] = vk::DescriptorImageInfo(*textures[i]->GetSampler(),
																							 textures[i]->GetImage().GetView(),
																							 textures[i]->GetImage().GetLayout());
		}

		SetWriteDescriptorSetDirty(index);
	}

	void Material::SetWriteDescriptorSetDirty(size_t index)
	{
		// if there's already a set in updates overwrite image info
		for (vk::WriteDescriptorSet& writeDescriptorSet : _updateImageWriteDescriptorSets)
		{
			if (writeDescriptorSet.dstBinding == _descriptorSetAllocation.ImageWriteDescriptorSets[index].dstBinding) { return; }
		}

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