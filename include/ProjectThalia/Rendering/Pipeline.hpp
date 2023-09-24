#pragma once

#include "RenderPass.hpp"
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
			Pipeline(const std::string&             name,
					 const std::vector<ShaderInfo>& shaderInfos,
					 const vk::Device&              device,
					 const RenderPass&              renderPass,
					 const Swapchain&               swapchain);

			void Destroy(vk::Device device);

			[[nodiscard]] const vk::Pipeline&       GetVkPipeline() const;
			[[nodiscard]] const vk::PipelineLayout& GetLayout() const;

		private:
			vk::Pipeline       _vkPipeline;
			vk::PipelineLayout _layout;

			std::vector<vk::ShaderModule> _shaderModules;
	};
}
