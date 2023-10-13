#pragma once

#include "ProjectThalia/Rendering/Vulkan/DeviceObject.hpp"
#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Image : DeviceObject
	{
		public:
			Image() = default;
			Image(const Device* device, const char* pixels, vk::DeviceSize pixelsSizeInBytes, vk::Extent3D extend);

			void TransitionLayout(vk::ImageLayout newLayout);
			void TransitionLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout newLayout);

			void Destroy() override;

			[[nodiscard]] const vk::Image&        GetVkImage() const;
			[[nodiscard]] const vk::ImageView&    GetView() const;
			[[nodiscard]] const vk::DeviceMemory& GetMemory() const;
			[[nodiscard]] vk::Format              GetFormat() const;
			[[nodiscard]] vk::ImageLayout         GetLayout() const;

		private:
			vk::Image        _vkImage;
			vk::ImageView    _view;
			vk::DeviceMemory _memory;

			vk::Format      _format = vk::Format::eR8G8B8A8Srgb;
			vk::ImageLayout _layout = vk::ImageLayout::eUndefined;
	};
}
