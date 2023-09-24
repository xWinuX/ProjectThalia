#pragma once

#include "PhysicalDevice.hpp"
#include "SDL_video.h"

#include <optional>
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering
{
	class Swapchain
	{
		public:
			Swapchain() = default;

			explicit Swapchain(const vk::Device&     device,
							   const PhysicalDevice& physicalDevice,
							   const vk::RenderPass& renderPass,
							   const vk::SurfaceKHR& surface,
							   vk::Extent2D          size);

			void Destroy(vk::Device device);

			[[nodiscard]] const vk::SwapchainKHR&             GetVkSwapchain() const;
			[[nodiscard]] const std::vector<vk::Image>&       GetImages() const;
			[[nodiscard]] const std::vector<vk::ImageView>&   GetImageViews() const;
			[[nodiscard]] const vk::Extent2D&                 GetExtend() const;
			[[nodiscard]] const std::vector<vk::Framebuffer>& GetFrameBuffers() const;

		private:
			vk::SwapchainKHR             _vkSwapchain;
			std::vector<vk::Image>       _images;
			std::vector<vk::ImageView>   _imageViews;
			std::vector<vk::Framebuffer> _frameBuffers;

			vk::Extent2D _extend = {};
	};

}
