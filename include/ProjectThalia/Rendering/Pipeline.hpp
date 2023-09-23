#pragma once

#include "Device.hpp"
#include "Swapchain.hpp"
#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering
{
	class Pipeline
	{
		public:
			struct ShaderInfo
			{
				public:
					std::string             path;
					vk::ShaderStageFlagBits shaderStage;
			};

		public:
			Pipeline() = default;
			Pipeline(const std::string&                         name,
					 const std::vector<ShaderInfo>&             shaderInfos,
					 const ProjectThalia::Rendering::Device&    device,
					 const ProjectThalia::Rendering::Swapchain& swapchain);

			[[nodiscard]] const vk::Pipeline&       GetVkPipeline() const;
			[[nodiscard]] const vk::PipelineLayout& GetLayout() const;
			[[nodiscard]] const vk::RenderPass&     GetRenderPass() const;

		private:
			vk::Pipeline       _vkPipeline;
			vk::PipelineLayout _layout;
			vk::RenderPass     _renderPass;

			std::vector<vk::ShaderModule> _shaderModules = std::vector<vk::ShaderModule>(2);
	};
}
