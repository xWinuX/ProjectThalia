#pragma once

#include "ProjectThalia/Rendering/Vulkan/DeviceObject.hpp"
#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Image : DeviceObject
	{
		public:
			Image(const Device* device, const char* pixels, vk::DeviceSize pixelsSizeInBytes, vk::Extent3D extend);

			void TransitionLayout(vk::ImageLayout newLayout);
			void TransitionLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout newLayout);

		private:
			vk::Image        _vkImage;
			vk::DeviceMemory _memory;

			vk::Format      _format = vk::Format::eR8G8B8A8Srgb;
			vk::ImageLayout _layout = vk::ImageLayout::eUndefined;
	};
}
