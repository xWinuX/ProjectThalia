#include "SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Debug/Log.hpp"

#include <set>
#include <utility>

#include "SplitEngine/Rendering/Vulkan/Instance.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	bool PhysicalDevice::IsQueueFamilyIndicesCompleted()
	{
		return std::ranges::all_of(_queueFamilyIndices, [](const std::optional<uint32_t>& family) { return family.has_value(); });
	}

	PhysicalDevice::PhysicalDevice(Instance& instance, std::vector<const char*> _requiredExtensions, std::vector<const char*> _requiredValidationLayers) :
		_instance(instance),
		_extensions(_requiredExtensions),
		_validationLayers(std::move(_requiredValidationLayers))
	{
		const std::vector<vk::PhysicalDevice> physicalDevices = instance.GetVkInstance().enumeratePhysicalDevices();
		for (const vk::PhysicalDevice& physicalDevice: physicalDevices)
		{
			// Check device type
			_properties = physicalDevice.getProperties();
			if (_properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) { continue; }

			// Check Extensions
			std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

			std::set<std::string> requiredExtensions = std::set<std::string>(_requiredExtensions.begin(), _requiredExtensions.end());

			for (const auto& extension: availableExtensions) { requiredExtensions.erase(extension.extensionName); }

			if (!requiredExtensions.empty()) { continue; }

			// Check queues
			std::ranges::fill(_queueFamilyIndices, std::optional<uint32_t>()); // Reset for each new physical device

			auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
			int  i                     = 0;
			for (const vk::QueueFamilyProperties& queueFamily: queueFamilyProperties)
			{
				// Graphics queue
				if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && !_queueFamilyIndices[CommandType::Graphics].has_value())
				{
					LOG("Graphics Family Index: {0}", i);
					_queueFamilyIndices[CommandType::Present] = i;
				}

				// Present queue
				if (physicalDevice.getSurfaceSupportKHR(i, instance.GetVkSurface()) && !_queueFamilyIndices[CommandType::Present].has_value())
				{
					LOG("Present Family Index: {0}", i);
					_queueFamilyIndices[CommandType::Present] = i;
				}

				// Compute queue
				if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute && !_queueFamilyIndices[CommandType::Compute].has_value())
				{
					LOG("Compute Family Index: {0}", i);
					_queueFamilyIndices[CommandType::Compute] = i;
				}

				if (IsQueueFamilyIndicesCompleted()) { break; }

				i++;
			}

			if (!IsQueueFamilyIndicesCompleted()) { continue; }

			// Check swap chain support
			_swapchainSupportDetails = SwapchainSupportDetails();

			_swapchainSupportDetails.Capabilities = physicalDevice.getSurfaceCapabilitiesKHR(instance.GetVkSurface());
			_swapchainSupportDetails.Formats      = physicalDevice.getSurfaceFormatsKHR(instance.GetVkSurface());
			_swapchainSupportDetails.PresentModes = physicalDevice.getSurfacePresentModesKHR(instance.GetVkSurface());

			if (_swapchainSupportDetails.Formats.empty() || _swapchainSupportDetails.PresentModes.empty()) { continue; }

			// Select device
			_vkPhysicalDevice = physicalDevice;
			LOG("Selected physical device: {0}", _properties.deviceName.data());
			break;
		}

		// Check if we found a compatible GPU
		if (_vkPhysicalDevice == VK_NULL_HANDLE) { ErrorHandler::ThrowRuntimeError("This device does not have any gpus meeting the applications requirements"); }

		// Select image format
		_imageFormat = _swapchainSupportDetails.Formats[0];
		for (const auto& availableFormat: _swapchainSupportDetails.Formats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) { _imageFormat = availableFormat; }
		}

		// Select depth image format
		_depthImageFormat = vk::Format::eD32Sfloat;
		for (vk::Format format: { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint })
		{
			vk::FormatProperties props = _vkPhysicalDevice.getFormatProperties(format);

			if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment)
			{
				_depthImageFormat = format;
				break;
			}
		}

		_device = std::make_unique<Device>(*this);
	}

	Instance& PhysicalDevice::GetInstance() const { return _instance; }

	Device& PhysicalDevice::GetDevice() const { return *_device.get(); }

	void PhysicalDevice::UpdateSwapchainSupportDetails(const vk::SurfaceKHR& surface)
	{
		_swapchainSupportDetails.Capabilities = _vkPhysicalDevice.getSurfaceCapabilitiesKHR(surface);
	}

	const vk::PhysicalDevice& PhysicalDevice::GetVkPhysicalDevice() const { return _vkPhysicalDevice; }

	const PhysicalDevice::QueueFamilyIndices& PhysicalDevice::GetQueueFamilyIndices() const { return _queueFamilyIndices; }

	const PhysicalDevice::SwapchainSupportDetails& PhysicalDevice::GetSwapchainSupportDetails() const { return _swapchainSupportDetails; }

	const std::vector<const char*>& PhysicalDevice::GetExtensions() const { return _extensions; }

	const std::vector<const char*>& PhysicalDevice::GetValidationLayers() const { return _validationLayers; }

	const vk::SurfaceFormatKHR& PhysicalDevice::GetImageFormat() const { return _imageFormat; }

	const vk::PhysicalDeviceProperties& PhysicalDevice::GetProperties() const { return _properties; }

	void PhysicalDevice::Destroy() { _device->Destroy(); }

	vk::Format PhysicalDevice::GetDepthImageFormat() const { return _depthImageFormat; }
}
