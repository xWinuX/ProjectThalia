#pragma once

#include "Allocator.hpp"
#include "DeviceObject.hpp"
#include "vulkan/vulkan.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	class Device;

	class Image : DeviceObject
	{
		public:
			struct CreateInfo
			{
				public:
					vk::Flags<vk::ImageUsageFlagBits>  Usage            = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
					vk::ImageLayout                    TransitionLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
					vk::Flags<vk::ImageAspectFlagBits> AspectMask       = vk::ImageAspectFlagBits::eColor;
					vk::Format                         Format           = vk::Format::eR8G8B8A8Srgb;
					VkMemoryPropertyFlags              RequiredFlags    = {};
			};

		public:
			Image() = default;
			Image(Device* device, const unsigned char* pixels, vk::DeviceSize pixelsSizeInBytes, vk::Extent3D extend, CreateInfo createInfo);

			void TransitionLayout(vk::ImageLayout newLayout);
			void TransitionLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout newLayout);

			void Destroy() override;

			[[nodiscard]] const vk::Image&       GetVkImage() const;
			[[nodiscard]] const vk::ImageView&   GetView() const;
			[[nodiscard]] const vk::ImageLayout& GetLayout() const;

		private:
			Allocator::ImageAllocation _imageAllocation;
			vk::ImageView              _view = VK_NULL_HANDLE;

			vk::ImageLayout _layout = vk::ImageLayout::eUndefined;
	};
}
