#include "ProjectThalia/Rendering/Pipeline.hpp"
#include "ProjectThalia/Application.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/IO/Stream.hpp"

namespace ProjectThalia::Rendering
{
	Pipeline::Pipeline(const std::string& name, const std::vector<ShaderInfo>& shaderInfos, const Device& device, const Swapchain& swapchain)
	{
		std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>(shaderInfos.size());
		_shaderModules.reserve(shaderInfos.size());

		for (int i = 0; i < shaderInfos.size(); ++i)
		{
			std::vector<char> shaderCode = IO::Stream::ReadRawAndClose(shaderInfos[i].path, IO::Binary);

			vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()));

			vk::ShaderModule shaderModule = VulkanContext::GetDevice().createShaderModule(createInfo);
			_shaderModules.push_back(shaderModule);

			vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = vk::PipelineShaderStageCreateInfo({},
																										shaderInfos[i].shaderStage,
																										_shaderModules[i],
																										"main");
			_shaderStages[i] = shaderStageCreateInfo;
		}

		Debug::Log::Info("After shader compiling");

		std::vector<vk::DynamicState> dynamicStates = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
		};

		vk::PipelineDynamicStateCreateInfo       dynamicStateCreateInfo     = vk::PipelineDynamicStateCreateInfo({}, dynamicStates);
		vk::PipelineVertexInputStateCreateInfo   vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo({}, 0, nullptr, 0, nullptr);
		vk::PipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo    = vk::PipelineInputAssemblyStateCreateInfo({},
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

		_layout = device.GetVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);

		vk::AttachmentDescription colorAttachment = vk::AttachmentDescription({},
																			  swapchain.GetImageFormat().format,
																			  vk::SampleCountFlagBits::e1,
																			  vk::AttachmentLoadOp::eClear,
																			  vk::AttachmentStoreOp::eStore,
																			  vk::AttachmentLoadOp::eDontCare,
																			  vk::AttachmentStoreOp::eDontCare,
																			  vk::ImageLayout::eUndefined,
																			  vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colorAttachmentReference = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
		vk::SubpassDescription  subpass                  = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &colorAttachmentReference);


		vk::SubpassDependency subpassDependency = vk::SubpassDependency(vk::SubpassExternal,
																		{},
																		vk::PipelineStageFlagBits::eColorAttachmentOutput,
																		vk::PipelineStageFlagBits::eColorAttachmentOutput,
																		{},
																		vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo({}, 1, &colorAttachment, 1, &subpass, 1, &subpassDependency);

		_renderPass = device.GetVkDevice().createRenderPass(renderPassCreateInfo);

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
																								   _renderPass,
																								   0,
																								   VK_NULL_HANDLE,
																								   -1);
		
		vk::ResultValue<vk::Pipeline> graphicsPipelineResult = device.GetVkDevice().createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo);
		if (graphicsPipelineResult.result != vk::Result::eSuccess) { ErrorHandler::ThrowRuntimeError("Failed to create graphics pipeline!"); }

		_vkPipeline = graphicsPipelineResult.value;
	}

	const vk::Pipeline& Pipeline::GetVkPipeline() const { return _vkPipeline; }

	const vk::PipelineLayout& Pipeline::GetLayout() const { return _layout; }

	const vk::RenderPass& Pipeline::GetRenderPass() const { return _renderPass; }
}