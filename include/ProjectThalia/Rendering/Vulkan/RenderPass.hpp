#pragma once

#include "DeviceObject.hpp"
#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class RenderPass final : DeviceObject
	{
		public:
			RenderPass() = default;
			explicit RenderPass(const Device* device);

			[[nodiscard]] const vk::RenderPass& GetVkRenderPass() const;

			void Destroy() override;

		private:
			vk::RenderPass _vkRenderPass;
	};
}
