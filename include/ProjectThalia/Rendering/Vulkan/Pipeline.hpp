#pragma once

#include "DescriptorSetManager.hpp"
#include "DeviceObject.hpp"
#include "spirv_common.hpp"

#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Pipeline final : DeviceObject
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

			Pipeline(Device* device, const std::string& name, const std::vector<ShaderInfo>& shaderInfos);

			void Destroy() override;

			[[nodiscard]] const vk::Pipeline&       GetVkPipeline() const;
			[[nodiscard]] const vk::PipelineLayout& GetLayout() const;
			[[nodiscard]] DescriptorSetManager&     GetDescriptorSetManager();

		private:
			vk::Pipeline       _vkPipeline;
			vk::PipelineLayout _layout;

			DescriptorSetManager _descriptorSetManager;

			std::vector<vk::ShaderModule> _shaderModules;

			[[nodiscard]] vk::Format GetFormatFromType(const spirv_cross::SPIRType& type) const;
	};
}
