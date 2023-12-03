#include "ProjectThalia/Rendering/Vulkan/Pipeline.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/IO/Stream.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"
#include "spirv_cross/spirv_cross.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Pipeline::Pipeline(Device* device, const std::string& name, const std::vector<ShaderInfo>& shaderInfos) :
		DeviceObject(device)
	{
		std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>(shaderInfos.size());
		_shaderModules.reserve(shaderInfos.size());

		// Descriptors
		std::vector<vk::DescriptorSetLayoutBinding> descriptorLayoutBindings;
		std::vector<vk::DescriptorPoolSize>         descriptorPoolSizes;
		std::vector<vk::WriteDescriptorSet>         writeDescriptorSets;

		// Vertex Input
		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
		vk::PipelineVertexInputStateCreateInfo           vertexInputStateCreateInfo {};

		for (int i = 0; i < shaderInfos.size(); i++)
		{
			std::vector<uint32_t> shaderCode = IO::Stream::ReadRawAndClose<uint32_t>(shaderInfos[i].path, IO::Binary);

			// Reflect shader and create Descriptor resources
			spirv_cross::Compiler spirvCompiler = spirv_cross::Compiler(shaderCode);

			spirv_cross::ShaderResources shaderResources = spirvCompiler.get_shader_resources();

			// Uniform Buffers
			std::unordered_map<vk::DescriptorType, const spirv_cross::SmallVector<spirv_cross::Resource>&> resourceMap = {
					{vk::DescriptorType::eUniformBuffer, shaderResources.uniform_buffers},
					{vk::DescriptorType::eCombinedImageSampler, shaderResources.sampled_images},
			};

			// Setup descriptor layout
			for (const auto& [type, resources] : resourceMap)
			{
				for (const spirv_cross::Resource& resource : resources)
				{
					uint32_t binding = spirvCompiler.get_decoration(resource.id, spv::DecorationBinding);

					vk::DescriptorSetLayoutBinding layoutBinding = vk::DescriptorSetLayoutBinding(binding, type, 1, shaderInfos[i].shaderStage);
					vk::DescriptorPoolSize         poolSize      = vk::DescriptorPoolSize(type, Device::MAX_FRAMES_IN_FLIGHT);

					vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet(VK_NULL_HANDLE, binding, 0,1, type, nullptr, nullptr, nullptr);
					const spirv_cross::SPIRType& resourceType = spirvCompiler.get_type(resource.base_type_id);

					switch (type)
					{
						case vk::DescriptorType::eUniformBuffer:
						{
							vk::DeviceSize uniformSize = 0;
							for (const auto& memberTypeID : resourceType.member_types)
							{
								const spirv_cross::SPIRType& memberType = spirvCompiler.get_type(memberTypeID);
								uniformSize += memberType.width;
							}
							vk::DescriptorBufferInfo* descriptorBufferInfo = new vk::DescriptorBufferInfo(VK_NULL_HANDLE, 0, uniformSize / 8);
							writeDescriptorSet.pBufferInfo                 = descriptorBufferInfo;
							break;
						}
						case vk::DescriptorType::eCombinedImageSampler:
						{
							vk::DescriptorImageInfo* descriptorImageInfo = new vk::DescriptorImageInfo({}, {}, {});
							writeDescriptorSet.pImageInfo                = descriptorImageInfo;
							break;
						}
					}


					descriptorLayoutBindings.push_back(layoutBinding);
					descriptorPoolSizes.push_back(poolSize);
					writeDescriptorSets.push_back(writeDescriptorSet);
				}
			}

			// Create shader module
			vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, shaderCode.size() * sizeof(uint32_t), shaderCode.data());

			vk::ShaderModule shaderModule = device->GetVkDevice().createShaderModule(createInfo);
			_shaderModules.push_back(shaderModule);

			// Create shader stage info
			vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({},
																										shaderInfos[i].shaderStage,
																										_shaderModules[i],
																										name.c_str());

			_shaderStages[i] = shaderStageCreateInfo;

			// Vertex shader specific init
			if (shaderInfos[i].shaderStage == vk::ShaderStageFlagBits::eVertex)
			{
				// Setup vertex input attributes
				uint32_t offset = 0;
				for (const auto& stageInput : spirvCompiler.get_shader_resources().stage_inputs)
				{
					uint32_t                     location = spirvCompiler.get_decoration(stageInput.id, spv::DecorationLocation);
					uint32_t                     binding  = spirvCompiler.get_decoration(stageInput.id, spv::DecorationBinding);
					const spirv_cross::SPIRType& type     = spirvCompiler.get_type(stageInput.base_type_id);
					vk::Format                   format   = GetFormatFromType(type);

					vk::VertexInputAttributeDescription vertexInputAttributeDescription = vk::VertexInputAttributeDescription(location,
																															  binding,
																															  format,
																															  offset);
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

		_descriptorSetManager = DescriptorSetManager(GetDevice(), descriptorLayoutBindings, descriptorPoolSizes, writeDescriptorSets, 10);

		std::vector<vk::DynamicState> dynamicStates = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
		};


		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo({}, dynamicStates);


		vk::PipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo({},
																													vk::PrimitiveTopology::eTriangleList,
																													vk::False);

		const vk::Extent2D& extend   = device->GetSwapchain().GetExtend();
		vk::Viewport        viewport = vk::Viewport(0,
                                             static_cast<float>(extend.height),
                                             static_cast<float>(extend.width),
                                             -static_cast<float>(extend.height),
                                             0.0f,
                                             1.0f);

		vk::Rect2D scissor = vk::Rect2D({0, 0}, extend);

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


		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState(vk::False,
																												vk::BlendFactor::eOne,
																												vk::BlendFactor::eZero,
																												vk::BlendOp::eAdd,
																												vk::BlendFactor::eOne,
																												vk::BlendFactor::eZero,
																												vk::BlendOp::eAdd,
																												vk::ColorComponentFlagBits::eR |
																														vk::ColorComponentFlagBits::eG |
																														vk::ColorComponentFlagBits::eB |
																														vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo({},
																												vk::False,
																												vk::LogicOp::eCopy,
																												1,
																												&colorBlendAttachmentState,
																												{0.0f, 0.0f, 0.0f, 0.0f});

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo({},
																							 1,
																							 &_descriptorSetManager.GetDescriptorSetLayout(),
																							 0,
																							 nullptr);

		_layout = device->GetVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo({},
																								   2,
																								   _shaderStages.data(),
																								   &vertexInputStateCreateInfo,
																								   &assemblyStateCreateInfo,
																								   nullptr,
																								   &viewportStateCreateInfo,
																								   &rasterizationStateCreateInfo,
																								   &multisampleStateCreateInfo,
																								   nullptr,
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

	vk::Format Pipeline::GetFormatFromType(const spirv_cross::SPIRType& type) const
	{
		switch (type.basetype)
		{
			case spirv_cross::SPIRType::Float:
				switch (type.vecsize)
				{
					case 1: return vk::Format::eR32Sfloat;
					case 2: return vk::Format::eR32G32Sfloat;
					case 3: return vk::Format::eR32G32B32Sfloat;
					case 4: return vk::Format::eR32G32B32A32Sfloat;
				}
				break;
		}

		return vk::Format::eUndefined;
	}

	const vk::Pipeline& Pipeline::GetVkPipeline() const { return _vkPipeline; }

	const vk::PipelineLayout& Pipeline::GetLayout() const { return _layout; }

	void Pipeline::Destroy()
	{
		_descriptorSetManager.Destroy();
		for (const vk::ShaderModule& item : _shaderModules) { Utility::DeleteDeviceHandle(GetDevice(), item); }
		Utility::DeleteDeviceHandle(GetDevice(), _layout);
		Utility::DeleteDeviceHandle(GetDevice(), _vkPipeline);
	}

	DescriptorSetManager& Pipeline::GetDescriptorSetManager() { return _descriptorSetManager; }
}