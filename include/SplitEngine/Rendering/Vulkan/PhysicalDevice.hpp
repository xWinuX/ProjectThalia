#pragma once

#include <vulkan/vulkan.hpp>

#include "QueueFamily.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	class Device;
	class Instance;

	class PhysicalDevice
	{
		public:
			struct SwapchainSupportDetails
			{
				public:
					vk::SurfaceCapabilitiesKHR        Capabilities;
					std::vector<vk::SurfaceFormatKHR> Formats;
					std::vector<vk::PresentModeKHR>   PresentModes;
			};

			struct QueueFamilyInfo
			{
				QueueType WantedCommandType = QueueType::MAX_VALUE;
				uint32_t          Index             = std::numeric_limits<uint32_t>::max();
				uint32_t          QueueCount        = 0;
			};

			typedef std::array<QueueFamilyInfo, static_cast<size_t>(QueueType::MAX_VALUE)> QueueFamilyInfos;

		public:
			bool     IsQueueFamilyIndicesCompleted();
			void     SearchQueues(const Instance& instance, const vk::PhysicalDevice& physicalDevice);
			explicit PhysicalDevice(Instance& instance, std::vector<const char*> _requiredExtensions, std::vector<const char*> _requiredValidationLayers);

			[[nodiscard]] Instance&                           GetInstance() const;
			[[nodiscard]] Device&                             GetDevice() const;
			[[nodiscard]] const vk::PhysicalDevice&           GetVkPhysicalDevice() const;
			[[nodiscard]] const QueueFamilyInfos&             GetQueueFamilyInfos() const;
			[[nodiscard]] const SwapchainSupportDetails&      GetSwapchainSupportDetails() const;
			[[nodiscard]] const std::vector<const char*>&     GetExtensions() const;
			[[nodiscard]] const std::vector<const char*>&     GetValidationLayers() const;
			[[nodiscard]] const vk::SurfaceFormatKHR&         GetImageFormat() const;
			[[nodiscard]] vk::Format                          GetDepthImageFormat() const;
			[[nodiscard]] const vk::PhysicalDeviceProperties& GetProperties() const;

			void Destroy();

			void UpdateSwapchainSupportDetails(const vk::SurfaceKHR& surface);

		private:
			Instance& _instance;

			QueueFamilyInfos _queueFamilyInfos{};

			vk::PhysicalDevice           _vkPhysicalDevice;
			std::unique_ptr<Device>      _device;
			SwapchainSupportDetails      _swapchainSupportDetails;
			vk::SurfaceFormatKHR         _imageFormat; // TODO: Does this really belong here?
			vk::Format                   _depthImageFormat{};
			std::vector<const char*>     _extensions;
			std::vector<const char*>     _validationLayers;
			vk::PhysicalDeviceProperties _properties;
	};
}
