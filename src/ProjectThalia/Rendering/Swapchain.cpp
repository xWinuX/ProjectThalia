#include "ProjectThalia/Rendering/Swapchain.hpp"

namespace ProjectThalia::Rendering
{
	ProjectThalia::Rendering::Swapchain::Swapchain(const vk::Device& device, const PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, vk::Extent2D size)
	{
		// Select surface format
		const PhysicalDevice::SwapchainSupportDetails& swapchainSupportDetails = physicalDevice.GetSwapchainSupportDetails();

		_imageFormat = swapchainSupportDetails.formats[0];
		for (const auto& availableFormat : swapchainSupportDetails.formats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				_imageFormat = availableFormat;
			}
		}

		// Select present mode
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		for (const auto& availablePresentMode : swapchainSupportDetails.presentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			{
				presentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
		}

		// Select swap extend
		const vk::SurfaceCapabilitiesKHR& capabilities = swapchainSupportDetails.capabilities;
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) { _extend = capabilities.currentExtent; }
		else
		{
			_extend.width  = std::clamp(size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			_extend.height = std::clamp(size.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		// Select image count
		uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
		if (swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapchainSupportDetails.capabilities.maxImageCount)
		{
			imageCount = swapchainSupportDetails.capabilities.maxImageCount;
		}

		// Create swap chain
		vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR({},
																					surface,
																					imageCount,
																					_imageFormat.format,
																					_imageFormat.colorSpace,
																					size,
																					1,
																					vk::ImageUsageFlagBits::eColorAttachment);

		const PhysicalDevice::QueueFamilyIndices& queueFamilyIndices = physicalDevice.GetQueueFamilyIndices();
		if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
		{
			std::vector<uint32_t> queueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};
			swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
			swapChainCreateInfo.setQueueFamilyIndices(queueFamilies);
		}
		else { swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive); }

		swapChainCreateInfo.setPreTransform(swapchainSupportDetails.capabilities.currentTransform);
		swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		swapChainCreateInfo.setPresentMode(presentMode);
		swapChainCreateInfo.setClipped(vk::True);
		swapChainCreateInfo.setOldSwapchain(VK_NULL_HANDLE);

		_vkSwapchain = device.createSwapchainKHR(swapChainCreateInfo);
		_images      = device.getSwapchainImagesKHR(_vkSwapchain);

		// Create image views
		_imageViews.resize(_images.size());

		for (int i = 0; i < _images.size(); ++i)
		{
			vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo({},
																				  _images[i],
																				  vk::ImageViewType::e2D,
																				  _imageFormat.format,
																				  {vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity,
																				   vk::ComponentSwizzle::eIdentity},
																				  {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

			_imageViews[i] = device.createImageView(imageViewCreateInfo);
		}
	}

	const vk::SwapchainKHR& Swapchain::GetVkSwapchain() const { return _vkSwapchain; }

	const vk::SurfaceFormatKHR& Swapchain::GetImageFormat() const { return _imageFormat; }

	const std::vector<vk::Image>& Swapchain::GetImages() const { return _images; }

	const std::vector<vk::ImageView>& Swapchain::GetImageViews() const { return _imageViews; }

	const vk::Extent2D& Swapchain::GetExtend() const { return _extend; }

	void Swapchain::Destroy(vk::Device device)
	{
		device.destroy(_vkSwapchain);
		for (const vk::ImageView& imageView : _imageViews) { device.destroy(imageView); }
	}
}
