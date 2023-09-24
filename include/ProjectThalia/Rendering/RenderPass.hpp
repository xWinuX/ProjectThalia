#pragma once

#include "Device.hpp"
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering
{

	class RenderPass
	{
		public:
			RenderPass() = default;
			RenderPass(const ProjectThalia::Rendering::Device& device, vk::Format format);

			[[nodiscard]] const vk::RenderPass& GetVkRenderPass() const;

		private:
			vk::RenderPass _vkRenderPass;
	};

}
