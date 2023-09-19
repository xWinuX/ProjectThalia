#pragma once

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

			Pipeline(const std::string& name, const std::vector<ShaderInfo>& shaderInfos);

		private:
			vk::Pipeline _pipeline = VK_NULL_HANDLE;

			std::vector<vk::ShaderModule> _shaderModules = std::vector<vk::ShaderModule>(2);
	};
}
