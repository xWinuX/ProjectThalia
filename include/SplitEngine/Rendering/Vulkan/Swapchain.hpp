#pragma once

#include "DeviceObject.hpp"
#include "Image.hpp"

#include <vulkan/vulkan.hpp>

namespace SplitEngine::Rendering::Vulkan
{
	class Device;

	class Swapchain final : DeviceObject
	{
		public:
			Swapchain() = default;

			explicit Swapchain(Device* device, const vk::SurfaceKHR& surface, vk::Extent2D size);

			void Destroy() override;

			[[nodiscard]] const vk::SwapchainKHR&             GetVkSwapchain() const;
			[[nodiscard]] const std::vector<vk::Image>&       GetImages() const;
			[[nodiscard]] const std::vector<vk::ImageView>&   GetImageViews() const;
			[[nodiscard]] const vk::Extent2D&                 GetExtend() const;
			[[nodiscard]] const std::vector<vk::Framebuffer>& GetFrameBuffers() const;

		private:
			vk::SwapchainKHR       _vkSwapchain;
			std::vector<vk::Image> _images;

			std::unique_ptr<Image> _depthImage;

			std::vector<vk::ImageView>   _imageViews;
			std::vector<vk::Framebuffer> _frameBuffers;

			vk::Extent2D _extend = {};
	};
}
