#pragma once

#include "vulkan/vulkan.hpp"
#include <optional>

namespace ProjectThalia::Rendering::Vulkan
{
	class PhysicalDevice
	{
		public:
			struct QueueFamilyIndices
			{
				public:
					std::optional<uint32_t> graphicsFamily;
					std::optional<uint32_t> presentFamily;

					[[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
			};

			struct SwapchainSupportDetails
			{
				public:
					vk::SurfaceCapabilitiesKHR        capabilities;
					std::vector<vk::SurfaceFormatKHR> formats;
					std::vector<vk::PresentModeKHR>   presentModes;
			};

		public:
			PhysicalDevice() = default;

			explicit PhysicalDevice(const vk::Instance&      instance,
									const vk::SurfaceKHR&    surface,
									std::vector<const char*> _requiredExtensions,
									std::vector<const char*> _requiredValidationLayers);

			[[nodiscard]] const vk::PhysicalDevice&       GetVkPhysicalDevice() const;
			[[nodiscard]] const QueueFamilyIndices&       GetQueueFamilyIndices() const;
			[[nodiscard]] const SwapchainSupportDetails&  GetSwapchainSupportDetails() const;
			[[nodiscard]] const std::vector<const char*>& GetExtensions() const;
			[[nodiscard]] const std::vector<const char*>& GetValidationLayers() const;
			[[nodiscard]] const vk::SurfaceFormatKHR&     GetImageFormat() const;

		private:
			vk::PhysicalDevice       _vkPhysicalDevice;
			QueueFamilyIndices       _queueFamilyIndices;
			SwapchainSupportDetails  _swapchainSupportDetails;
			vk::SurfaceFormatKHR     _imageFormat; // TODO: Does this really belong here?
			std::vector<const char*> _extensions;
			std::vector<const char*> _validationLayers;
	};
}