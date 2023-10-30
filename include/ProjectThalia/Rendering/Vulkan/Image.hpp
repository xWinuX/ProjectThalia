#pragma once

#include "Allocator.hpp"
#include "ProjectThalia/Rendering/Vulkan/DeviceObject.hpp"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Image : DeviceObject
	{
		public:
			Image() = default;
			Image(Device* device, const char* pixels, vk::DeviceSize pixelsSizeInBytes, vk::Extent3D extend);

			void TransitionLayout(vk::ImageLayout newLayout);
			void TransitionLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout newLayout);

			void Destroy() override;

			[[nodiscard]] const vk::Image&     GetVkImage() const;
			[[nodiscard]] const vk::ImageView& GetView() const;
			[[nodiscard]] vk::Format           GetFormat() const;
			[[nodiscard]] vk::ImageLayout      GetLayout() const;

		private:
			Allocator::ImageAllocation _imageAllocation;
			vk::ImageView              _view;

			vk::Format      _format = vk::Format::eR8G8B8A8Srgb;
			vk::ImageLayout _layout = vk::ImageLayout::eUndefined;
	};
}
