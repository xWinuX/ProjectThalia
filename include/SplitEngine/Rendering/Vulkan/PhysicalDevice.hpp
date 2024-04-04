#pragma once

#include "vulkan/vulkan.hpp"

#include <optional>

#include "CommandType.hpp"


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

			typedef std::array<std::optional<uint32_t>, CommandType::MAX_VALUE> QueueFamilyIndices;

		public:
			bool     IsQueueFamilyIndicesCompleted();
			explicit PhysicalDevice(Instance& instance, std::vector<const char*> _requiredExtensions, std::vector<const char*> _requiredValidationLayers);

			[[nodiscard]] Instance&                                                          GetInstance() const;
			[[nodiscard]] Device&                                                            GetDevice() const;
			[[nodiscard]] const vk::PhysicalDevice&                                          GetVkPhysicalDevice() const;
			[[nodiscard]] const QueueFamilyIndices& GetQueueFamilyIndices() const;
			[[nodiscard]] const SwapchainSupportDetails&                                     GetSwapchainSupportDetails() const;
			[[nodiscard]] const std::vector<const char*>&                                    GetExtensions() const;
			[[nodiscard]] const std::vector<const char*>&                                    GetValidationLayers() const;
			[[nodiscard]] const vk::SurfaceFormatKHR&                                        GetImageFormat() const;
			[[nodiscard]] vk::Format                                                         GetDepthImageFormat() const;
			[[nodiscard]] const vk::PhysicalDeviceProperties&                                GetProperties() const;

			void Destroy();

			void UpdateSwapchainSupportDetails(const vk::SurfaceKHR& surface);

		private:
			Instance& _instance;

			QueueFamilyIndices _queueFamilyIndices;

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
