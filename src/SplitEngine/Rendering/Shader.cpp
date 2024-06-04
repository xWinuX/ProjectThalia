#include "SplitEngine/Rendering/Shader.hpp"
#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Rendering/Texture2D.hpp"
#include "SplitEngine/Rendering/Vulkan/Instance.hpp"
#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"

#include <filesystem>
#include <vector>

#define PRIVATE_FIF_SELECTOR(descriptorPtr, fifVar, attribute) fifVar == -1 ? descriptorPtr->attribute.Get() : descriptorPtr->attribute[fifVar]

#define PRIVATE_EXECUTE_SHARED(functionCall) \
if (descriptor->SharedID != -1) \
{ \
	for (auto& propertyEntry: _sharedProperties[descriptor->SharedID].Propertieses) \
	{ \
		if (propertyEntry.Property != nullptr) \
		{ \
			propertyEntry.Property->functionCall; \
		} \
	} \
}


namespace SplitEngine::Rendering
{
	Shader::Properties Shader::_globalProperties{};

	uint32_t                                             Shader::Properties::_idCounter        = 0;
	std::vector<Shader::Properties::SharedPropertyEntry> Shader::Properties::_sharedProperties = std::vector<Shader::Properties::SharedPropertyEntry>();

	bool Shader::_globalPropertiesDefined = false;

	std::vector<Vulkan::Pipeline::ShaderInfo> Shader::CreateShaderInfos()
	{
		std::vector<Vulkan::Pipeline::ShaderInfo> shaderInfos{};
		const ShaderParserSettings&               shaderParserSettings = _device->GetPhysicalDevice().GetInstance().GetShaderParserSettings();

		shaderInfos.reserve(_shaderPaths.size());

		// Get shader infos
		for (std::filesystem::path& file: _shaderPaths)
		{
			if (!file.has_extension()) { ErrorHandler::ThrowRuntimeError(std::format("Failed to parse shader files! {0}", file.string())); }

			std::string shaderTypeString = file.extension().string();
			ShaderType  shaderType       = ShaderType::Vertex;

			if (shaderTypeString == shaderParserSettings.VertexShaderFileExtension) { shaderType = ShaderType::Vertex; }
			if (shaderTypeString == shaderParserSettings.FragmentShaderFileExtension) { shaderType = ShaderType::Fragment; }
			if (shaderTypeString == shaderParserSettings.ComputeShaderFileExtension) { shaderType = ShaderType::Compute; }

			file.concat(shaderParserSettings.SpirvFileExtension);

			shaderInfos.push_back({ file, shaderType });
		}

		return shaderInfos;
	}

	Shader::Shader(const CreateInfo& createInfo) :
		_shaderPaths(createInfo.ShaderPaths),
		_device(&Vulkan::Instance::Get().GetPhysicalDevice().GetDevice()),
		_pipeline(_device, "main", CreateShaderInfos()),
		_shaderProperties(Properties(this, &_pipeline.GetPerPipelineDescriptorSetAllocation()))
	{
		if (!_globalPropertiesDefined)
		{
			_globalProperties        = Properties(this, &Vulkan::Pipeline::GetGlobalDescriptorSetAllocation());
			_globalPropertiesDefined = true;
		}
	}

	Vulkan::Pipeline& Shader::GetPipeline() { return _pipeline; }

	Shader::~Shader() { _pipeline.Destroy(); }

	void Shader::PushConstant(const vk::CommandBuffer& commandBuffer, ShaderType shaderType, uint32_t index, void* data) const
	{
		const Vulkan::Pipeline::PushConstantInfo& pushConstantInfo = _pipeline.GetPushConstantInfo(shaderType, index);

		commandBuffer.pushConstants(_pipeline.GetLayout(), Vulkan::Pipeline::GetShaderStageFromShaderType(shaderType), pushConstantInfo.Offset, pushConstantInfo.Range, data);
	}

	void Shader::Update() { _shaderProperties.Update(); }

	void Shader::Bind(const vk::CommandBuffer& commandBuffer, uint32_t frameInFlight) const
	{
		_pipeline.Bind(commandBuffer);
		_pipeline.BindDescriptorSets(commandBuffer, _shaderProperties._descriptorSetAllocation, 1, 0, nullptr, frameInFlight);
	}

	Shader::Properties& Shader::GetProperties() { return _shaderProperties; }

	Shader::Properties& Shader::GetGlobalProperties() { return _globalProperties; }


	void Shader::BindGlobal(const vk::CommandBuffer& commandBuffer, uint32_t frameInFlight) const
	{
		_pipeline.BindDescriptorSets(commandBuffer, _globalProperties._descriptorSetAllocation, 0, 0, nullptr, frameInFlight);
	}

	void Shader::UpdateGlobal() { _globalProperties.Update(); }

	void Shader::Properties::SetSharedWriteDescriptorsDirty(Vulkan::Descriptor* descriptor, uint32_t frameInFlight)
	{
		for (auto& propertyEntry: _sharedProperties[descriptor->SharedID].PropertyEntries)
		{
			if (propertyEntry.Property != nullptr) { propertyEntry.Property->SetWriteDescriptorSetDirty(propertyEntry.BindingPoint, frameInFlight); }
		}
	}

	void Shader::Properties::SetTexture(uint32_t bindingPoint, const Texture2D& texture)
	{
		Vulkan::Descriptor* descriptor = GetDescriptor(bindingPoint);

		descriptor->ImageInfos.Get()[0] = vk::DescriptorImageInfo(*texture.GetSampler(), texture.GetImage().GetView(), texture.GetImage().GetLayout());

		SetWriteDescriptorSetDirty(bindingPoint);

		if (descriptor->SharedID != -1) { SetSharedWriteDescriptorsDirty(descriptor); }
	}

	void Shader::Properties::SetTextures(uint32_t bindingPoint, size_t offset, std::vector<std::unique_ptr<Texture2D>>& textures)
	{
		Vulkan::Descriptor*                   descriptor           = GetDescriptor(bindingPoint);
		std::vector<vk::DescriptorImageInfo>& descriptorImageInfos = descriptor->ImageInfos.Get();
		for (int i = 0; i < textures.size(); ++i)
		{
			descriptorImageInfos[i + offset] = vk::DescriptorImageInfo(*textures[i]->GetSampler(), textures[i]->GetImage().GetView(), textures[i]->GetImage().GetLayout());
		}

		SetWriteDescriptorSetDirty(bindingPoint);

		if (descriptor->SharedID != -1) { SetSharedWriteDescriptorsDirty(descriptor); }
	}

	void Shader::Properties::SetTextures(const uint32_t bindingPoint, const size_t offset, std::vector<AssetHandle<Texture2D>>& textures)
	{
		Vulkan::Descriptor*                   descriptor           = GetDescriptor(bindingPoint);
		std::vector<vk::DescriptorImageInfo>& descriptorImageInfos = descriptor->ImageInfos.Get();
		for (int i = 0; i < textures.size(); ++i)
		{
			descriptorImageInfos[i + offset] = vk::DescriptorImageInfo(*textures[i]->GetSampler(), textures[i]->GetImage().GetView(), textures[i]->GetImage().GetLayout());
		}

		SetWriteDescriptorSetDirty(bindingPoint);

		if (descriptor->SharedID != -1) { SetSharedWriteDescriptorsDirty(descriptor); }
	}

	void Shader::Properties::SetBuffer(const uint32_t bindingPoint, const Vulkan::Buffer& buffer, const size_t offset, const size_t range, const uint32_t frameInFlight)
	{
		Vulkan::Descriptor* descriptor = GetDescriptor(bindingPoint);

		vk::DescriptorBufferInfo& descriptorBufferInfo = PRIVATE_FIF_SELECTOR(descriptor, frameInFlight, BufferInfos);

		descriptorBufferInfo.buffer = buffer.GetVkBuffer();
		descriptorBufferInfo.offset = offset;
		descriptorBufferInfo.range  = range;

		if (frameInFlight == -1) { descriptor->BufferPtrs.Set(buffer.GetMappedData<std::byte>() + offset); }
		else { descriptor->BufferPtrs[frameInFlight] = buffer.GetMappedData<std::byte>() + offset; }

		SetWriteDescriptorSetDirty(bindingPoint, frameInFlight);

		if (descriptor->SharedID != -1) { SetSharedWriteDescriptorsDirty(descriptor, frameInFlight); }
	}

	void Shader::Properties::OverrideBuffer(uint32_t bindingPoint, Vulkan::Buffer&& buffer)
	{
		Vulkan::Descriptor* descriptor = GetDescriptor(bindingPoint);

		descriptor->Buffer = std::move(buffer);

		for (vk::DescriptorBufferInfo& descriptorBufferInfo: descriptor->BufferInfos.GetDataVector()) { descriptorBufferInfo.buffer = descriptor->Buffer.GetVkBuffer(); }

		SetWriteDescriptorSetDirty(bindingPoint);

		if (descriptor->SharedID != -1) { SetSharedWriteDescriptorsDirty(descriptor); }
	}

	void Shader::Properties::OverrideBufferPtrs(uint32_t bindingPoint, Vulkan::Buffer& buffer)
	{
		Vulkan::Descriptor* descriptor = GetDescriptor(bindingPoint);

		std::vector<std::byte*>& bytes = descriptor->BufferPtrs.GetDataVector();

		for (int i = 0; i < bytes.size(); ++i) { bytes[i] = buffer.GetMappedData<std::byte>() + descriptor->BufferInfos[i].offset; }
	}

	Vulkan::Buffer& Shader::Properties::GetBuffer(uint32_t bindingPoint) const { return GetDescriptor(bindingPoint)->Buffer; }

	const vk::DescriptorBufferInfo& Shader::Properties::GetBufferInfo(const uint32_t bindingPoint, const uint32_t frameInFlight) const
	{
		Vulkan::Descriptor* descriptor = GetDescriptor(bindingPoint);
		return PRIVATE_FIF_SELECTOR(descriptor, frameInFlight, BufferInfos);
	}

	Shader::Properties::Properties(Shader* shader, Vulkan::DescriptorSetAllocator::Allocation* descriptorSetAllocation) :
		_shader(shader),
		_descriptorSetAllocation(descriptorSetAllocation)
	{
		_id = _idCounter++;

		for (auto& descriptorEntry: _descriptorSetAllocation->DescriptorEntries)
		{
			uint32_t sharedId = descriptorEntry.Descriptor->SharedID;
			if (sharedId == -1) { continue; }

			if (sharedId == _sharedProperties.size()) { _sharedProperties.push_back({}); }

			SharedPropertyEntry& sharedPropertyEntry = _sharedProperties[sharedId];

			if (!sharedPropertyEntry.AvailableStack.IsEmpty())
			{
				sharedPropertyEntry.PropertyEntries[sharedPropertyEntry.AvailableStack.Pop()] = { descriptorEntry.BindingPoint, this };
			}
			else { sharedPropertyEntry.PropertyEntries.emplace_back(descriptorEntry.BindingPoint, this); }

			_sharedPtrRemoveIndex.push_back(sharedPropertyEntry.PropertyEntries.size() - 1);
		}
	}


	Shader::Properties::~Properties()
	{
		if (_descriptorSetAllocation != nullptr)
		{
			uint32_t i = 0;
			for (auto& descriptorEntry: _descriptorSetAllocation->DescriptorEntries)
			{
				uint32_t sharedId = descriptorEntry.Descriptor->SharedID;
				if (sharedId == -1) { continue; }

				uint32_t removeIndex                                     = _sharedPtrRemoveIndex[i];
				_sharedProperties[sharedId].PropertyEntries[removeIndex] = { -1u, nullptr };
				_sharedProperties[sharedId].AvailableStack.Push(removeIndex);
			}
			_sharedPtrRemoveIndex.clear();
		}
	}

	Vulkan::Descriptor* Shader::Properties::GetDescriptor(uint32_t bindingPoint) const
	{
		const uint32_t index = _descriptorSetAllocation->SparseDescriptorLookup[bindingPoint];
		return _descriptorSetAllocation->DescriptorEntries[index].Descriptor;
	}

	void Shader::Properties::SetWriteDescriptorSetDirty(uint32_t bindingPoint, uint32_t frameInFlight)
	{
		const uint32_t      index      = _descriptorSetAllocation->SparseDescriptorLookup[bindingPoint];
		Vulkan::Descriptor* descriptor = GetDescriptor(bindingPoint);
		for (int i = 0; i < Vulkan::Device::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (descriptor->SingleInstance) { _updateWriteDescriptorSets.push_back(_descriptorSetAllocation->WriteDescriptorSets[index][i]); }
			else
			{
				_updateWriteDescriptorSets.push_back(frameInFlight == -1
					                                     ? _descriptorSetAllocation->WriteDescriptorSets[index].Get()
					                                     : _descriptorSetAllocation->WriteDescriptorSets[index][frameInFlight]);
				break;
			}
		}
	}

	void Shader::Properties::Update()
	{
		if (!_updateWriteDescriptorSets.empty())
		{
			Vulkan::Instance::Get().GetPhysicalDevice().GetDevice().GetVkDevice().updateDescriptorSets(_updateWriteDescriptorSets, nullptr);
		}

		_updateWriteDescriptorSets.clear();
	}
}
