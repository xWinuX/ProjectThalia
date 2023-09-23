#include "ProjectThalia/Rendering/PhysicalDevice.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"

#include <set>

namespace ProjectThalia::Rendering
{
	PhysicalDevice::PhysicalDevice(const vk::Instance&      instance,
								   const vk::SurfaceKHR&    surface,
								   std::vector<const char*> _requiredExtensions,
								   std::vector<const char*> _requiredValidationLayers) :
		_extensions(_requiredExtensions),
		_validationLayers(_requiredValidationLayers)
	{
		std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
		for (const vk::PhysicalDevice& physicalDevice : physicalDevices)
		{
			// Check device type
			vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
			if (deviceProperties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) { continue; }

			// Check Extensions
			std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

			std::set<std::string> requiredExtensions = std::set<std::string>(_requiredExtensions.begin(), _requiredExtensions.end());

			for (const auto& extension : availableExtensions) { requiredExtensions.erase(extension.extensionName); }

			if (!requiredExtensions.empty()) { continue; }

			// Check queues
			_queueFamilyIndices = QueueFamilyIndices();

			auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
			int  i                     = 0;
			for (const vk::QueueFamilyProperties& queueFamily : queueFamilyProperties)
			{
				unsigned int presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);

				if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && !_queueFamilyIndices.graphicsFamily.has_value())
				{
					Debug::Log::Info(std::format("graphics index: {0}", i));
					_queueFamilyIndices.graphicsFamily = i;
				}

				if (presentSupport && !_queueFamilyIndices.presentFamily.has_value())
				{
					Debug::Log::Info(std::format("present index: {0}", i));
					_queueFamilyIndices.presentFamily = i;
				}

				if (_queueFamilyIndices.isComplete()) { break; }

				i++;
			}

			if (!_queueFamilyIndices.isComplete()) { continue; }


			// Check swap chain support
			_swapchainSupportDetails = SwapchainSupportDetails();

			_swapchainSupportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
			_swapchainSupportDetails.formats      = physicalDevice.getSurfaceFormatsKHR(surface);
			_swapchainSupportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

			if (_swapchainSupportDetails.formats.empty() || _swapchainSupportDetails.presentModes.empty()) { continue; }

			// Select device
			_vkPhysicalDevice = physicalDevice;
			Debug::Log::Info(std::format("Selected physical device: {0}", deviceProperties.deviceName.data()));
			break;
		}

		// Check if we found a compatible GPU
		if (_vkPhysicalDevice == VK_NULL_HANDLE) { ErrorHandler::ThrowRuntimeError("This device does not have any gpus meeting the applications requirements"); }
	}

	const vk::PhysicalDevice& PhysicalDevice::GetVkPhysicalDevice() const { return _vkPhysicalDevice; }

	const PhysicalDevice::QueueFamilyIndices& PhysicalDevice::GetQueueFamilyIndices() const { return _queueFamilyIndices; }

	const PhysicalDevice::SwapchainSupportDetails& PhysicalDevice::GetSwapchainSupportDetails() const { return _swapchainSupportDetails; }

	const std::vector<const char*>& PhysicalDevice::GetExtensions() const { return _extensions; }

	const std::vector<const char*>& PhysicalDevice::GetValidationLayers() const { return _validationLayers; }
}
