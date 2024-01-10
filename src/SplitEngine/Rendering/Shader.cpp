#include "SplitEngine/Rendering/Shader.hpp"
#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Rendering/Vulkan/Context.hpp"
#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"

#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

namespace SplitEngine::Rendering
{
	Shader::Properties Shader::_globalProperties {};

	bool Shader::_globalPropertiesDefined = false;

	Shader::Shader(const CreateInfo& createInfo) :
		_shaderPath(createInfo.ShaderPath)
	{
		_shaderProperties = Properties(this, &_pipeline.GetPerPipelineDescriptorSetAllocation());

		if (!_globalPropertiesDefined)
		{
			_globalProperties        = Properties(this, &Vulkan::Pipeline::GetGlobalDescriptorSetAllocation());
			_globalPropertiesDefined = true;
		}

		std::vector<std::filesystem::path> files;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(_shaderPath))
		{
			bool hasSpvExtension = entry.path().extension() == std::format(".{0}", Application::GetApplicationInfo().SpirvFileExtension);
			if (std::filesystem::is_regular_file(entry.path()) && hasSpvExtension) { files.push_back(entry.path()); }
		}

		std::vector<Vulkan::Pipeline::ShaderInfo> shaderInfos;
		shaderInfos.reserve(files.size());

		// Get shader infos
		for (const auto& file : files)
		{
			size_t lastDotPosition       = file.string().rfind('.');
			size_t secondLastDotPosition = -1;

			// Find seconds last point
			for (size_t i = lastDotPosition - 1; i > 0; i--)
			{
				if (file.string()[i] == '.')
				{
					secondLastDotPosition = i;
					break;
				}
			}

			if (lastDotPosition == std::string::npos || secondLastDotPosition == std::string::npos)
			{
				ErrorHandler::ThrowRuntimeError(std::format("Failed to parse shader files! {0}", _shaderPath));
			}

			std::string                  shaderTypeString = file.string().substr(secondLastDotPosition + 1, lastDotPosition - secondLastDotPosition - 1);
			Vulkan::Pipeline::ShaderType shaderType       = Vulkan::Pipeline::ShaderType::Vertex;


			if (shaderTypeString == Application::GetApplicationInfo().VertexShaderFileExtension) { shaderType = Vulkan::Pipeline::ShaderType::Vertex; }
			if (shaderTypeString == Application::GetApplicationInfo().FragmentShaderFileExtension) { shaderType = Vulkan::Pipeline::ShaderType::Fragment; }

			shaderInfos.push_back({file.string(), shaderType});
		}

		_pipeline = Vulkan::Pipeline(Vulkan::Context::GetDevice(), "main", shaderInfos);
	}

	Vulkan::Pipeline& Shader::GetPipeline() { return _pipeline; }

	Shader::~Shader() { _pipeline.Destroy(); }

	void Shader::Update() { _shaderProperties.Update(); }

	void Shader::Bind(vk::CommandBuffer& commandBuffer)
	{
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.GetVkPipeline());
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
										 _pipeline.GetLayout(),
										 1,
										 1,
										 &_shaderProperties._descriptorSetAllocation->DescriptorSet,
										 0,
										 nullptr);
	}

	Shader::Properties& Shader::GetProperties() { return _shaderProperties; }

	Shader::Properties& Shader::GetGlobalProperties() { return _globalProperties; }

	void Shader::BindGlobal(vk::CommandBuffer& commandBuffer)
	{
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
										 _pipeline.GetLayout(),
										 0,
										 1,
										 &_globalProperties._descriptorSetAllocation->DescriptorSet,
										 0,
										 nullptr);
	}

	void Shader::UpdateGlobal() { _globalProperties.Update(); }

	void Shader::Properties::SetTexture(uint32_t bindingPoint, const Texture2D& texture)
	{
		_descriptorSetAllocation->ImageInfos[bindingPoint][0] = vk::DescriptorImageInfo(*texture.GetSampler(),
																						texture.GetImage().GetView(),
																						texture.GetImage().GetLayout());
		SetWriteDescriptorSetDirty(bindingPoint);
	}

	void Shader::Properties::SetTextures(uint32_t bindingPoint, size_t offset, std::vector<std::unique_ptr<Texture2D>>& textures)
	{
		for (int i = 0; i < textures.size(); ++i)
		{
			LOG("image infos {0}", _descriptorSetAllocation->ImageInfos.size());
			LOG("image infos {0}", _descriptorSetAllocation->ImageInfos[bindingPoint].size());
			_descriptorSetAllocation->ImageInfos[bindingPoint][i + offset] = vk::DescriptorImageInfo(*textures[i]->GetSampler(),
																									 textures[i]->GetImage().GetView(),
																									 textures[i]->GetImage().GetLayout());
		}

		SetWriteDescriptorSetDirty(bindingPoint);
	}

	void Shader::Properties::SetTextures(uint32_t bindingPoint, size_t offset, std::vector<AssetHandle<Texture2D>>& textures)
	{
		// if there's already a set in updates overwrite image info
		for (vk::WriteDescriptorSet& writeDescriptorSet : _updateImageWriteDescriptorSets)
		{
			if (writeDescriptorSet.dstBinding == _descriptorSetAllocation->ImageWriteDescriptorSets[bindingPoint].dstBinding) { return; }
		}

		_updateImageWriteDescriptorSets.push_back(_descriptorSetAllocation->ImageWriteDescriptorSets[bindingPoint]);
	}

	Shader::Properties::Properties(Shader* shader, Vulkan::DescriptorSetAllocator::Allocation* descriptorSetAllocation) :
		_shader(shader),
		_descriptorSetAllocation(descriptorSetAllocation)
	{}

	void Shader::Properties::SetWriteDescriptorSetDirty(size_t index)
	{
		// if there's already a set in updates overwrite image info
		for (vk::WriteDescriptorSet& writeDescriptorSet : _updateImageWriteDescriptorSets)
		{
			if (writeDescriptorSet.dstBinding == _descriptorSetAllocation->ImageWriteDescriptorSets[index].dstBinding) { return; }
		}

		_updateImageWriteDescriptorSets.push_back(_descriptorSetAllocation->ImageWriteDescriptorSets[index]);
	}

	void Shader::Properties::Update()
	{
		if (!_updateImageWriteDescriptorSets.empty())
		{
			Vulkan::Context::GetDevice()->GetVkDevice().updateDescriptorSets(_updateImageWriteDescriptorSets, nullptr);
		}

		_updateImageWriteDescriptorSets.clear();
	}

}
