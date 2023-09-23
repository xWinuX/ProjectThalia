#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>


namespace ProjectThalia::Rendering
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

			explicit PhysicalDevice(const vk::Instance&             instance,
									const vk::SurfaceKHR&           surface,
									std::vector<const char*> _requiredExtensions,
									std::vector<const char*> _requiredValidationLayers);

			[[nodiscard]] const vk::PhysicalDevice&      GetVkPhysicalDevice() const;
			[[nodiscard]] const QueueFamilyIndices&      GetQueueFamilyIndices() const;
			[[nodiscard]] const SwapchainSupportDetails& GetSwapchainSupportDetails() const;
			const std::vector<const char*>&              GetExtensions() const;
			const std::vector<const char*>&              GetValidationLayers() const;

		private:
			vk::PhysicalDevice       _vkPhysicalDevice;
			QueueFamilyIndices      _queueFamilyIndices;
			SwapchainSupportDetails _swapchainSupportDetails;
			std::vector<const char*> _extensions;
			std::vector<const char*> _validationLayers;
	};

}