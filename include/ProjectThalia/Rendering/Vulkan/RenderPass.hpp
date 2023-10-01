#pragma once

#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class RenderPass
	{
		public:
			RenderPass() = default;
			RenderPass(const Device& device);

			[[nodiscard]] const vk::RenderPass& GetVkRenderPass() const;

			void Destroy(vk::Device device);

		private:
			vk::RenderPass _vkRenderPass;
	};
}
