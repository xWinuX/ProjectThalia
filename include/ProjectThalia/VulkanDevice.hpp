#pragma once
#include "vulkan/vulkan.h"
#include <optional>
#include <vector>

namespace ProjectThalia
{
	class VulkanDevice
	{
		public:
			struct QueueFamilyIndices {
					std::optional<uint32_t> graphicsFamily;
					std::optional<uint32_t> presentFamily;

					[[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
			};

			VulkanDevice() = default;
			VulkanDevice(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilyIndices queueFamilyIndices);

			static VulkanDevice FindAndSetupBestDevice(const VkInstance&               vulkanInstance,
													   const VkSurfaceKHR&             surfaceKhr,
											   const std::vector<const char*>& validationLayers,
											   const std::vector<const char*>& deviceExtensions);

			[[nodiscard]] const VkPhysicalDevice&   GetPhysicalDevice() const;
			[[nodiscard]] const VkDevice&           GetDevice() const;
			[[nodiscard]] const QueueFamilyIndices& GetQueueFamilyIndices() const;

		private:
			VkPhysicalDevice   _physicalDevice = VK_NULL_HANDLE;
			VkDevice           _device         = VK_NULL_HANDLE;
			QueueFamilyIndices _queueFamilyIndices;

			static bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& deviceExtensions);
			static bool IsDeviceSuitable(const VkPhysicalDevice& physicalDevice, const std::vector<const char*>& extensions);
			static QueueFamilyIndices
			FindQueueFamilies(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const std::vector<VkQueueFamilyProperties>& queueFamilies);
	};
}