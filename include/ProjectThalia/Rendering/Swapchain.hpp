#pragma once

#include "Device.hpp"
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
			explicit Swapchain(const Device& device, const vk::SurfaceKHR& surface, vk::Extent2D size);

			[[nodiscard]] const vk::SwapchainKHR&           GetVkSwapchain() const;
			[[nodiscard]] const vk::SurfaceFormatKHR&       GetImageFormat() const;
			[[nodiscard]] const std::vector<vk::Image>&     GetImages() const;
			[[nodiscard]] const std::vector<vk::ImageView>& GetImageViews() const;
			[[nodiscard]] const vk::Extent2D&               GetExtend() const;

		private:
			vk::SwapchainKHR           _vkSwapchain;
			vk::SurfaceFormatKHR       _imageFormat;
			std::vector<vk::Image>     _images;
			std::vector<vk::ImageView> _imageViews;
			vk::Extent2D               _extend = {};
	};

}
