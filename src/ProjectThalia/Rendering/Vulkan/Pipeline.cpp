#include "ProjectThalia/Rendering/Vulkan/Pipeline.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/IO/Stream.hpp"
#include "ProjectThalia/Rendering/Vertex.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	Pipeline::Pipeline(const vk::Device&              device,
					   const RenderPass&              renderPass,
					   const Swapchain&               swapchain,
					   const std::string&             name,
					   const std::vector<ShaderInfo>& shaderInfos)
	{
		std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>(shaderInfos.size());
		_shaderModules.reserve(shaderInfos.size());

		for (int i = 0; i < shaderInfos.size(); ++i)
		{
			std::vector<char> shaderCode = IO::Stream::ReadRawAndClose(shaderInfos[i].path, IO::Binary);

			vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()));

			vk::ShaderModule shaderModule = device.createShaderModule(createInfo);
			_shaderModules.push_back(shaderModule);

			vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({},
																										shaderInfos[i].shaderStage,
																										_shaderModules[i],
																										"main");
			_shaderStages[i]                                        = shaderStageCreateInfo;
		}
		
		std::vector<vk::DynamicState> dynamicStates = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
		};


		for (auto& item : VertexPosition2DColor::VertexInputAttributeDescriptions)
		{
			LOG("item: {0}", std::to_string((int)item.format));
		}

		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo({}, dynamicStates);
		vk::PipelineVertexInputStateCreateInfo
				vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo({},
																					1,
																					&VertexPosition2DColor::VertexInputBindingDescription,
																					VertexPosition2DColor::VertexInputAttributeDescriptions.size(),
																					VertexPosition2DColor::VertexInputAttributeDescriptions.data());

		vk::PipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo({},
																													vk::PrimitiveTopology::eTriangleList,
																													vk::False);

		vk::Viewport viewport = vk::Viewport(0,
											 0,
											 static_cast<float>(swapchain.GetExtend().width),
											 static_cast<float>(swapchain.GetExtend().height),
											 0.0f,
											 1.0f);

		vk::Rect2D scissor = vk::Rect2D({0, 0}, swapchain.GetExtend());

		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo({}, 1, &viewport, 1, &scissor);

		vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo({},
																														 vk::False,
																														 vk::False,
																														 vk::PolygonMode::eFill,
																														 vk::CullModeFlagBits::eBack,
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

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo({}, 0, nullptr, 0, nullptr);

		_layout = device.createPipelineLayout(pipelineLayoutCreateInfo);

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
																								   renderPass.GetVkRenderPass(),
																								   0,
																								   VK_NULL_HANDLE,
																								   -1);

		vk::ResultValue<vk::Pipeline> graphicsPipelineResult = device.createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo);
		if (graphicsPipelineResult.result != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create graphics pipeline!"); }

		_vkPipeline = graphicsPipelineResult.value;
	}

	const vk::Pipeline& Pipeline::GetVkPipeline() const { return _vkPipeline; }

	const vk::PipelineLayout& Pipeline::GetLayout() const { return _layout; }

	void Pipeline::Destroy(vk::Device device)
	{
		for (const vk::ShaderModule& item : _shaderModules) { Utility::DeleteDeviceHandle(device, item); }
		Utility::DeleteDeviceHandle(device, _layout);
		Utility::DeleteDeviceHandle(device, _vkPipeline);
	}
}