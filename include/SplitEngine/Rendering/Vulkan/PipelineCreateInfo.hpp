#pragma once

#include <vulkan/vulkan.hpp>

namespace SplitEngine::Rendering::Vulkan
{
	struct PipelineCreateInfo
	{
		vk::PipelineInputAssemblyStateCreateInfo AssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, vk::False);

		vk::PipelineRasterizationStateCreateInfo RasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo({},
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

		vk::PipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo({},
		                                                                                                           vk::SampleCountFlagBits::e1,
		                                                                                                           vk::False,
		                                                                                                           1.0f,
		                                                                                                           nullptr,
		                                                                                                           vk::False,
		                                                                                                           vk::False);

		vk::PipelineColorBlendAttachmentState ColorBlendAttachmentState = vk::PipelineColorBlendAttachmentState(vk::True,
		                                                                                                        vk::BlendFactor::eSrcAlpha,
		                                                                                                        vk::BlendFactor::eOneMinusSrcAlpha,
		                                                                                                        vk::BlendOp::eAdd,
		                                                                                                        vk::BlendFactor::eOne,
		                                                                                                        vk::BlendFactor::eZero,
		                                                                                                        vk::BlendOp::eAdd,
		                                                                                                        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		                                                                                                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo({},
		                                                                                                        vk::False,
		                                                                                                        vk::LogicOp::eCopy,
		                                                                                                        1,
		                                                                                                        nullptr,
		                                                                                                        { 0.0f, 0.0f, 0.0f, 0.0f });

		vk::PipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo({},
		                                                                                                              vk::True,
		                                                                                                              vk::True,
		                                                                                                              vk::CompareOp::eLess,
		                                                                                                              vk::False,
		                                                                                                              vk::False,
		                                                                                                              {},
		                                                                                                              {},
		                                                                                                              0.0,
		                                                                                                              1.0f);
	};
}
