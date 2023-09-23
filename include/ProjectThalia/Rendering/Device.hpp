#pragma once

#include "PhysicalDevice.hpp"
#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering
{

	class Device
	{
		public:
			explicit Device(PhysicalDevice& physicalDevice);

			[[nodiscard]] const vk::Device&     GetVkDevice() const;
			[[nodiscard]] const PhysicalDevice& GetPhysicalDevice() const;
			[[nodiscard]] const vk::Queue&      GetGraphicsQueue() const;
			[[nodiscard]] const vk::Queue&      GetPresentQueue() const;

		private:
			vk::Device      _vkDevice;
			PhysicalDevice& _physicalDevice;

			vk::Queue _graphicsQueue;
			vk::Queue _presentQueue;
	};

}
