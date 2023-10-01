#pragma once

#include "RenderPass.hpp"
#include "Swapchain.hpp"

#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

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
			Pipeline(const Device&              device,
					 const std::string&             name,
					 const std::vector<ShaderInfo>& shaderInfos);

			void Destroy(vk::Device device);

			[[nodiscard]] const vk::Pipeline&       GetVkPipeline() const;
			[[nodiscard]] const vk::PipelineLayout& GetLayout() const;

		private:
			vk::Pipeline       _vkPipeline;
			vk::PipelineLayout _layout;

			std::vector<vk::ShaderModule> _shaderModules;
	};
}
