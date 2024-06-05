#include "SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp"

#include <map>

#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Debug/Log.hpp"

#include <set>
#include <utility>

#include "SplitEngine/Rendering/Vulkan/Instance.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	bool PhysicalDevice::IsQueueFamilyIndicesCompleted()
	{
		return std::ranges::all_of(_queueFamilyInfos, [](const QueueFamilyInfo& family) { return family.Index != std::numeric_limits<uint32_t>::max() && family.QueueCount != 0; });
	}

	void PhysicalDevice::SearchQueues(const Instance& instance, const vk::PhysicalDevice& physicalDevice)
	{
		const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		// in the first iteration try to fit every command type into a seperate queue, in the second Iteration fill out the remaining queues
		for (int tryIteration = 0; tryIteration < 2; ++tryIteration)
		{
			for (int i = 0; i < queueFamilyProperties.size(); ++i)
			{
				const vk::QueueFamilyProperties& queueFamily = queueFamilyProperties[i];

				// Graphics queue
				if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && _queueFamilyInfos[static_cast<size_t>(QueueType::Graphics)].Index == std::numeric_limits<
					    uint32_t>::max())
				{
					LOG("Graphics Family Index: {0}", i);
					_queueFamilyInfos[static_cast<size_t>(QueueType::Graphics)].WantedCommandType = QueueType::Graphics;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Graphics)].Index             = i;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Graphics)].QueueCount        = queueFamily.queueCount;
					if (tryIteration == 0) { continue; }
				}

				// Compute queue
				if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute && _queueFamilyInfos[static_cast<size_t>(QueueType::Compute)].Index == std::numeric_limits<
					    uint32_t>::max())
				{
					LOG("Compute Family Index: {0}", i);
					_queueFamilyInfos[static_cast<size_t>(QueueType::Compute)].WantedCommandType = QueueType::Compute;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Compute)].Index             = i;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Compute)].QueueCount        = queueFamily.queueCount;
					if (tryIteration == 0) { continue; }
				}

				// Present queue
				if (physicalDevice.getSurfaceSupportKHR(i, instance.GetVkSurface()) && _queueFamilyInfos[static_cast<size_t>(QueueType::Present)].Index == std::numeric_limits<
					    uint32_t>::max())
				{
					LOG("Present Family Index: {0}", i);
					_queueFamilyInfos[static_cast<size_t>(QueueType::Present)].WantedCommandType = QueueType::Present;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Present)].Index             = i;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Present)].QueueCount        = queueFamily.queueCount;
					if (tryIteration == 0) { continue; }
				}

				// Transfer
				if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer && _queueFamilyInfos[static_cast<size_t>(QueueType::Transfer)].Index == std::numeric_limits<
					    uint32_t>::max())
				{
					LOG("Transfer Family Index: {0}", i);
					_queueFamilyInfos[static_cast<size_t>(QueueType::Transfer)].WantedCommandType = QueueType::Transfer;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Transfer)].Index             = i;
					_queueFamilyInfos[static_cast<size_t>(QueueType::Transfer)].QueueCount        = queueFamily.queueCount;
					if (tryIteration == 0) { continue; }
				}

				if (IsQueueFamilyIndicesCompleted()) { return; }
			}
		}
	}

	PhysicalDevice::PhysicalDevice(Instance& instance, std::vector<const char*> _requiredExtensions, std::vector<const char*> _requiredValidationLayers) :
		_instance(instance),
		_extensions(_requiredExtensions),
		_validationLayers(std::move(_requiredValidationLayers))
	{
		const std::vector<vk::PhysicalDevice> physicalDevices = instance.GetVkInstance().enumeratePhysicalDevices();

		LOG("Available Devices");
		for (vk::PhysicalDevice physicalDevice: physicalDevices)
		{
			vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
			LOG("ID {0} - {1}", properties.deviceID, properties.deviceName.data());
		}

		for (int i = 0; i < 2; i++)
		{
			uint32_t gpuDeviceId = i == 0 ? _instance.GetRenderingSettings().GPUDeviceID : -1u;

			std::map<size_t, vk::PhysicalDevice> physicalDeviceRating = std::map<size_t, vk::PhysicalDevice>();
			for (const vk::PhysicalDevice& physicalDevice: physicalDevices)
			{
				// Check device type
				_properties = physicalDevice.getProperties();

				if (gpuDeviceId != -1u && _properties.deviceID != gpuDeviceId) { continue; }

				// Check Extensions
				std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

				std::set<std::string> requiredExtensions = std::set<std::string>(_requiredExtensions.begin(), _requiredExtensions.end());

				for (const auto& extension: availableExtensions) { requiredExtensions.erase(extension.extensionName); }

				if (!requiredExtensions.empty()) { continue; }

				// Check queues
				std::ranges::fill(_queueFamilyInfos, QueueFamilyInfo()); // Reset for each new physical device

				SearchQueues(instance, physicalDevice);

				if (!IsQueueFamilyIndicesCompleted()) { continue; }

				// Check swap chain support
				_swapchainSupportDetails = SwapchainSupportDetails();

				_swapchainSupportDetails.Capabilities = physicalDevice.getSurfaceCapabilitiesKHR(instance.GetVkSurface());
				_swapchainSupportDetails.Formats      = physicalDevice.getSurfaceFormatsKHR(instance.GetVkSurface());
				_swapchainSupportDetails.PresentModes = physicalDevice.getSurfacePresentModesKHR(instance.GetVkSurface());

				if (_swapchainSupportDetails.Formats.empty() || _swapchainSupportDetails.PresentModes.empty()) { continue; }

				size_t points = 0;
				if (_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) { points += 100; }

				// Select device
				physicalDeviceRating[points] = physicalDevice;
			}

			// Check if we found a compatible GPU
			if (physicalDeviceRating.empty())
			{
				if (i > 0) { ErrorHandler::ThrowRuntimeError("This device does not have any gpus meeting the applications requirements"); }
				else
				{
					if (gpuDeviceId != -1u) { LOG_ERROR("Couldn't find GPU with the ID {0}", gpuDeviceId); }
					continue;
				}
			}

			// Select top GPU
			_vkPhysicalDevice = physicalDeviceRating.rbegin()->second;

			break;
		}

		LOG("Selected physical device: {0}", _vkPhysicalDevice.getProperties().deviceName.data());

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

	const PhysicalDevice::QueueFamilyInfos& PhysicalDevice::GetQueueFamilyInfos() const { return _queueFamilyInfos; }

	const PhysicalDevice::SwapchainSupportDetails& PhysicalDevice::GetSwapchainSupportDetails() const { return _swapchainSupportDetails; }

	const std::vector<const char*>& PhysicalDevice::GetExtensions() const { return _extensions; }

	const std::vector<const char*>& PhysicalDevice::GetValidationLayers() const { return _validationLayers; }

	const vk::SurfaceFormatKHR& PhysicalDevice::GetImageFormat() const { return _imageFormat; }

	const vk::PhysicalDeviceProperties& PhysicalDevice::GetProperties() const { return _properties; }

	void PhysicalDevice::Destroy() { _device->Destroy(); }

	vk::Format PhysicalDevice::GetDepthImageFormat() const { return _depthImageFormat; }
}
