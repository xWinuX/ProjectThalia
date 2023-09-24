#pragma once

#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering
{

	class RenderPass
	{
		public:
			RenderPass() = default;
			RenderPass(const vk::Device& device, vk::Format format);

			[[nodiscard]] const vk::RenderPass& GetVkRenderPass() const;

			void Destroy(vk::Device device);

		private:
			vk::RenderPass _vkRenderPass;
	};

}
