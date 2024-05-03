#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"

#include "spirv_cross/spirv_cross.hpp"
#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/IO/Stream.hpp"
#include "SplitEngine/Rendering/Vulkan/Utility.hpp"
#include "SplitEngine/Utility/String.hpp"

#include <set>

namespace SplitEngine::Rendering::Vulkan
{
	DescriptorSetAllocator             Pipeline::_globalDescriptorManager{};
	DescriptorSetAllocator::Allocation Pipeline::_globalDescriptorSetAllocation;
	bool                               Pipeline::_globalDescriptorsProcessed = false;

	Pipeline::Pipeline(Device* device, const std::string& name, const std::vector<ShaderInfo>& shaderInfos) :
		DeviceObject(device)
	{
		std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>(shaderInfos.size());
		_shaderModules.reserve(shaderInfos.size());

		// Descriptors
		std::vector<DescriptorSetAllocator::CreateInfo> _descriptorSetInfos = std::vector<DescriptorSetAllocator::CreateInfo>(3);

		// Vertex Input
		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(0);
		vk::PipelineVertexInputStateCreateInfo           vertexInputStateCreateInfo{};

		const RenderingSettings& renderingSettings = GetDevice()->GetPhysicalDevice().GetInstance().GetRenderingSettings();

		// Check if input is a compute shader
		bool isComputeShader = std::ranges::any_of(shaderInfos, [](const ShaderInfo& shaderInfo) { return shaderInfo.shaderStage == ShaderType::Compute; });
		if (isComputeShader && shaderInfos.size() > 1) { ErrorHandler::ThrowRuntimeError(std::format("Compute shaders can't have more than 1 file {0}", shaderInfos[0].path)); }

		_bindPoint = isComputeShader ? vk::PipelineBindPoint::eCompute : vk::PipelineBindPoint::eGraphics;

		for (int i = 0; i < shaderInfos.size(); i++)
		{
			std::vector<uint32_t> shaderCode = IO::Stream::ReadRawAndClose<uint32_t>(shaderInfos[i].path, IO::Binary);

			// Reflect shader and create Descriptor resources
			spirv_cross::Compiler spirvCompiler = spirv_cross::Compiler(shaderCode);

			spirv_cross::ShaderResources shaderResources = spirvCompiler.get_shader_resources();

			// Uniform Buffers
			std::unordered_map<vk::DescriptorType, const spirv_cross::SmallVector<spirv_cross::Resource>&> resourceMap = {
				{ vk::DescriptorType::eUniformBuffer, shaderResources.uniform_buffers },
				{ vk::DescriptorType::eStorageBuffer, shaderResources.storage_buffers },
				{ vk::DescriptorType::eCombinedImageSampler, shaderResources.sampled_images },
			};

			// Setup descriptor layout
			for (const auto& [type, resources]: resourceMap)
			{
				for (const spirv_cross::Resource& resource: resources)
				{
					// Check if we already have a layout binding with the same binding index if true add the current shader stage to its shader stage mask
					uint32_t binding = spirvCompiler.get_decoration(resource.id, spv::DecorationBinding);
					uint32_t set     = spirvCompiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

					// Exit if a set higher than 2 is specified (it isn't supported)
					if (set > 2)
					{
						ErrorHandler::ThrowRuntimeError(std::format("set decoration in shader {0} can't be higher than 2 (0 = global set, 1 = pipeline set, 2 = material set)",
						                                            shaderInfos[i].path));
					}

					// Skip global descriptor if we've already processed it
					if (set == 0 && _globalDescriptorsProcessed) { continue; }

					DescriptorSetAllocator::CreateInfo& descriptorSetInfo = _descriptorSetInfos[set];

					// If we already processed the binding we just add the shader stage to it and move on to the next resource
					if (descriptorSetInfo.Bindings.contains(binding))
					{
						for (vk::DescriptorSetLayoutBinding& descriptorLayoutBinding: descriptorSetInfo.DescriptorLayoutBindings)
						{
							if (descriptorLayoutBinding.binding == binding)
							{
								descriptorLayoutBinding.stageFlags |= static_cast<vk::ShaderStageFlagBits>(shaderInfos[i].shaderStage);
								break;
							}
						}
						continue;
					}

					DescriptorSetAllocator::DescriptorCreateInfo descriptorCreateInfo{};

					const spirv_cross::SPIRType& resourceBaseType = spirvCompiler.get_type(resource.base_type_id);
					const spirv_cross::SPIRType& resourceType     = spirvCompiler.get_type(resource.type_id);

					// Parse name to find modifiers
					descriptorCreateInfo.Name          = resource.name;
					std::vector<std::string> splitName = SplitEngine::Utility::String::Split(descriptorCreateInfo.Name, renderingSettings.ShaderBufferModDelimiter, 0);

					for (std::string& split: splitName)
					{
						if (!descriptorCreateInfo.SingleInstance)
						{
							descriptorCreateInfo.SingleInstance = std::ranges::find(renderingSettings.ShaderPropertySingleInstanceModPrefixes, split) != renderingSettings.
							                                      ShaderPropertySingleInstanceModPrefixes.end();
						}

						if (!descriptorCreateInfo.DeviceLocal)
						{
							descriptorCreateInfo.DeviceLocal = std::ranges::find(renderingSettings.ShaderBufferDeviceLocalModPrefixes, split) != renderingSettings.
							                                   ShaderBufferDeviceLocalModPrefixes.end();
						}

						if (!descriptorCreateInfo.Cached)
						{
							descriptorCreateInfo.Cached = std::ranges::find(renderingSettings.ShaderBufferCacheModPrefixes, split) != renderingSettings.ShaderBufferCacheModPrefixes
							                              .end();
						}

						if (!descriptorCreateInfo.Shared)
						{
							descriptorCreateInfo.Shared = std::ranges::find(renderingSettings.ShaderPropertySharedModPrefixes, split) != renderingSettings.
							                              ShaderPropertySharedModPrefixes.end();
						}

						if (!descriptorCreateInfo.NoAllocation)
						{
							descriptorCreateInfo.NoAllocation = std::ranges::find(renderingSettings.ShaderBufferNoAllocModPrefixes, split) != renderingSettings.
							                                    ShaderBufferNoAllocModPrefixes.end();
						}
					}

					// Create binding
					uint32_t descriptorCount = 1;
					if (!resourceType.array.empty()) { descriptorCount = resourceType.array[0]; }

					// Bind everywhere if set is global or if property is shared
					vk::ShaderStageFlagBits shaderStageFlagBits = descriptorCreateInfo.Shared || set == 0
						                                              ? vk::ShaderStageFlagBits::eAll
						                                              : static_cast<vk::ShaderStageFlagBits>(shaderInfos[i].shaderStage);
					vk::DescriptorSetLayoutBinding layoutBinding = vk::DescriptorSetLayoutBinding(binding, type, descriptorCount, shaderStageFlagBits);

					switch (type)
					{
						case vk::DescriptorType::eStorageBuffer:
						case vk::DescriptorType::eUniformBuffer:
						{
							vk::DeviceSize uniformSize = spirvCompiler.get_declared_struct_size(resourceBaseType);

							descriptorSetInfo.WriteDescriptorSets.emplace_back();

							size_t offset = 0;
							for (int j = 0; j < Device::MAX_FRAMES_IN_FLIGHT; ++j)
							{
								vk::DeviceSize minAlignment = type == vk::DescriptorType::eStorageBuffer
									                              ? device->GetPhysicalDevice().GetProperties().limits.minStorageBufferOffsetAlignment
									                              : device->GetPhysicalDevice().GetProperties().limits.minUniformBufferOffsetAlignment;

								vk::DeviceSize padding = minAlignment - (uniformSize % minAlignment);
								padding                = padding == minAlignment ? 0 : padding;

								descriptorSetInfo.WriteDescriptorSets.back().emplace_back(VK_NULL_HANDLE, binding, 0, descriptorCount, type, nullptr, nullptr, nullptr);
								descriptorSetInfo.WriteDescriptorSets.back().back().pBufferInfo = new vk::DescriptorBufferInfo(VK_NULL_HANDLE, offset, uniformSize + padding);

								if (!descriptorCreateInfo.SingleInstance) { offset += uniformSize + padding; }
							}
							break;
						}
						case vk::DescriptorType::eCombinedImageSampler:
							descriptorSetInfo.WriteDescriptorSets.emplace_back();
							for (int j = 0; j < Device::MAX_FRAMES_IN_FLIGHT; ++j)
							{
								descriptorSetInfo.WriteDescriptorSets.back().emplace_back(VK_NULL_HANDLE, binding, 0, descriptorCount, type, nullptr, nullptr, nullptr);
							}
							break;
					}

					descriptorSetInfo.Bindings.insert(binding);
					descriptorSetInfo.DescriptorPoolSizes.emplace_back(type, descriptorCount * Device::MAX_FRAMES_IN_FLIGHT);
					descriptorSetInfo.DescriptorLayoutBindings.push_back(layoutBinding);
					descriptorSetInfo.DescriptorCreateInfos.push_back(descriptorCreateInfo);
				}
			}

			// Create shader module
			vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, shaderCode.size() * sizeof(uint32_t), shaderCode.data());

			vk::ShaderModule shaderModule = device->GetVkDevice().createShaderModule(createInfo);
			_shaderModules.push_back(shaderModule);

			// Create shader stage info
			vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({},
			                                                                                            static_cast<vk::ShaderStageFlagBits>(shaderInfos[i].shaderStage),
			                                                                                            _shaderModules[i],
			                                                                                            name.c_str());

			_shaderStages[i] = shaderStageCreateInfo;


			// Vertex shader specific init
			if (shaderInfos[i].shaderStage == ShaderType::Vertex)
			{
				// Setup vertex input attributes
				uint32_t offset = 0;
				for (const auto& stageInput: spirvCompiler.get_shader_resources().stage_inputs)
				{
					uint32_t                     location = spirvCompiler.get_decoration(stageInput.id, spv::DecorationLocation);
					uint32_t                     binding  = spirvCompiler.get_decoration(stageInput.id, spv::DecorationBinding);
					const spirv_cross::SPIRType& type     = spirvCompiler.get_type(stageInput.base_type_id);
					vk::Format                   format   = GetFormatFromType(type);


					vk::VertexInputAttributeDescription vertexInputAttributeDescription = vk::VertexInputAttributeDescription(location, binding, format, offset);

					vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);

					offset += (type.width * type.vecsize) / 8;
				}


				// Setup pipeline vertex input state
				vk::VertexInputBindingDescription vertexInputBindingDescription = vk::VertexInputBindingDescription(0, offset, vk::VertexInputRate::eVertex);
				vertexInputStateCreateInfo                                      = vk::PipelineVertexInputStateCreateInfo({},
					1,
					&vertexInputBindingDescription,
					vertexInputAttributeDescriptions.size(),
					vertexInputAttributeDescriptions.data());
			}
		}

		if (!_globalDescriptorsProcessed)
		{
			_globalDescriptorManager       = DescriptorSetAllocator(GetDevice(), _descriptorSetInfos[0], 1);
			_globalDescriptorSetAllocation = _globalDescriptorManager.AllocateDescriptorSet();
			_globalDescriptorsProcessed    = true;
		}

		_perPipelineDescriptorSetManager    = DescriptorSetAllocator(GetDevice(), _descriptorSetInfos[1], 1);
		_perPipelineDescriptorSetAllocation = _perPipelineDescriptorSetManager.AllocateDescriptorSet();

		_perInstanceDescriptorSetManager = DescriptorSetAllocator(GetDevice(), _descriptorSetInfos[2], 10);

		_descriptorSetLayouts.push_back(_globalDescriptorManager.GetDescriptorSetLayout());
		_descriptorSetLayouts.push_back(_perPipelineDescriptorSetManager.GetDescriptorSetLayout());
		_descriptorSetLayouts.push_back(_perInstanceDescriptorSetManager.GetDescriptorSetLayout());


		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo({}, _descriptorSetLayouts, nullptr);
		_layout                                               = device->GetVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);

		if (!isComputeShader)
		{
			std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor, };

			vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo({}, dynamicStates);

			vk::PipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, vk::False);

			const vk::Extent2D& extend   = device->GetSwapchain().GetExtend();
			vk::Viewport        viewport = vk::Viewport(0, static_cast<float>(extend.height), static_cast<float>(extend.width), -static_cast<float>(extend.height), 0.0f, 1.0f);

			vk::Rect2D scissor = vk::Rect2D({ 0, 0 }, extend);

			vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo({}, 1, &viewport, 1, &scissor);

			vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo({},
				vk::False,
				vk::False,
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eNone,
				vk::FrontFace::eClockwise,
				vk::False,
				0.0f,
				0.0f,
				0.0f,
				1.0f);

			vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo({},
			                                                                                                           vk::SampleCountFlagBits::e1,
			                                                                                                           vk::False,
			                                                                                                           1.0f,
			                                                                                                           nullptr,
			                                                                                                           vk::False,
			                                                                                                           vk::False);

			vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState(vk::True,
			                                                                                                        vk::BlendFactor::eSrcAlpha,
			                                                                                                        vk::BlendFactor::eOneMinusSrcAlpha,
			                                                                                                        vk::BlendOp::eAdd,
			                                                                                                        vk::BlendFactor::eOne,
			                                                                                                        vk::BlendFactor::eZero,
			                                                                                                        vk::BlendOp::eAdd,
			                                                                                                        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
			                                                                                                        | vk::ColorComponentFlagBits::eB |
			                                                                                                        vk::ColorComponentFlagBits::eA);

			vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo({},
			                                                                                                        vk::False,
			                                                                                                        vk::LogicOp::eCopy,
			                                                                                                        1,
			                                                                                                        &colorBlendAttachmentState,
			                                                                                                        { 0.0f, 0.0f, 0.0f, 0.0f });

			vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo({},
				vk::True,
				vk::True,
				vk::CompareOp::eLess,
				vk::False,
				vk::False,
				{},
				{},
				0.0,
				1.0f);

			vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo({},
			                                                                                           2,
			                                                                                           _shaderStages.data(),
			                                                                                           &vertexInputStateCreateInfo,
			                                                                                           &assemblyStateCreateInfo,
			                                                                                           nullptr,
			                                                                                           &viewportStateCreateInfo,
			                                                                                           &rasterizationStateCreateInfo,
			                                                                                           &multisampleStateCreateInfo,
			                                                                                           &depthStencilStateCreateInfo,
			                                                                                           &colorBlendStateCreateInfo,
			                                                                                           &dynamicStateCreateInfo,
			                                                                                           _layout,
			                                                                                           device->GetRenderPass().GetVkRenderPass(),
			                                                                                           0,
			                                                                                           VK_NULL_HANDLE,
			                                                                                           -1);

			vk::ResultValue<vk::Pipeline> graphicsPipelineResult = device->GetVkDevice().createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo);
			if (graphicsPipelineResult.result != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create graphics pipeline!"); }

			_vkPipeline = graphicsPipelineResult.value;
		}
		else
		{
			vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo({}, _shaderStages[0], _layout);

			vk::ResultValue<vk::Pipeline> computePipelineResult = device->GetVkDevice().createComputePipeline(VK_NULL_HANDLE, computePipelineCreateInfo);
			if (computePipelineResult.result != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create graphics pipeline!"); }

			_vkPipeline = computePipelineResult.value;
		}
	}

	void Pipeline::Bind(const vk::CommandBuffer& commandBuffer) const { commandBuffer.bindPipeline(_bindPoint, _vkPipeline); }

	void Pipeline::BindDescriptorSets(const vk::CommandBuffer& commandBuffer,
	                                  const uint32_t           descriptorSetCount,
	                                  const vk::DescriptorSet* descriptorSets,
	                                  const uint32_t           firstSet,
	                                  uint32_t                 dynamicOffsetCount,
	                                  uint32_t*                dynamicOffsets) const
	{
		commandBuffer.bindDescriptorSets(_bindPoint, _layout, firstSet, descriptorSetCount, descriptorSets, dynamicOffsetCount, dynamicOffsets);
	}

	void Pipeline::BindDescriptorSets(const vk::CommandBuffer&            commandBuffer,
	                                  DescriptorSetAllocator::Allocation* descriptorSetAllocation,
	                                  const uint32_t                      firstSet,
	                                  const uint32_t                      dynamicOffsetCount,
	                                  uint32_t*                           dynamicOffsets,
	                                  uint32_t                            frameInFlight) const
	{
		commandBuffer.bindDescriptorSets(_bindPoint,
		                                 _layout,
		                                 firstSet,
		                                 1,
		                                 frameInFlight == -1 ? &descriptorSetAllocation->DescriptorSets.Get() : &descriptorSetAllocation->DescriptorSets[frameInFlight],
		                                 dynamicOffsetCount,
		                                 dynamicOffsets);
	}

	vk::Format Pipeline::GetFormatFromType(const spirv_cross::SPIRType& type)
	{
		switch (type.basetype)
		{
			case spirv_cross::SPIRType::Float:
				switch (type.vecsize)
				{
					case 1:
						return vk::Format::eR32Sfloat;
					case 2:
						return vk::Format::eR32G32Sfloat;
					case 3:
						return vk::Format::eR32G32B32Sfloat;
					case 4:
						return vk::Format::eR32G32B32A32Sfloat;
				}
				break;
			case spirv_cross::SPIRType::Int:
				switch (type.vecsize)
				{
					case 1:
						return vk::Format::eR32Sint;
					case 2:
						return vk::Format::eR32G32Sint;
					case 3:
						return vk::Format::eR32G32B32Sint;
					case 4:
						return vk::Format::eR32G32B32A32Sint;
				}
				break;
			case spirv_cross::SPIRType::UInt:
				switch (type.vecsize)
				{
					case 1:
						return vk::Format::eR32Uint;
					case 2:
						return vk::Format::eR32G32Uint;
					case 3:
						return vk::Format::eR32G32B32Uint;
					case 4:
						return vk::Format::eR32G32B32A32Uint;
				}
		}

		return vk::Format::eUndefined;
	}

	const vk::Pipeline& Pipeline::GetVkPipeline() const { return _vkPipeline; }

	const vk::PipelineLayout& Pipeline::GetLayout() const { return _layout; }

	void Pipeline::Destroy()
	{
		_perPipelineDescriptorSetManager.DeallocateDescriptorSet(_perPipelineDescriptorSetAllocation);

		_perInstanceDescriptorSetManager.Destroy();
		_perPipelineDescriptorSetManager.Destroy();

		for (const vk::ShaderModule& item: _shaderModules) { Utility::DeleteDeviceHandle(GetDevice(), item); }
		Utility::DeleteDeviceHandle(GetDevice(), _layout);
		Utility::DeleteDeviceHandle(GetDevice(), _vkPipeline);
	}

	DescriptorSetAllocator::Allocation& Pipeline::GetGlobalDescriptorSetAllocation() { return _globalDescriptorSetAllocation; }

	DescriptorSetAllocator::Allocation& Pipeline::GetPerPipelineDescriptorSetAllocation() { return _perPipelineDescriptorSetAllocation; }

	DescriptorSetAllocator::Allocation Pipeline::AllocatePerInstanceDescriptorSet() { return _perInstanceDescriptorSetManager.AllocateDescriptorSet(); }

	void Pipeline::DeallocatePerInstanceDescriptorSet(DescriptorSetAllocator::Allocation& descriptorSetAllocation)
	{
		_perInstanceDescriptorSetManager.DeallocateDescriptorSet(descriptorSetAllocation);
	}
}
