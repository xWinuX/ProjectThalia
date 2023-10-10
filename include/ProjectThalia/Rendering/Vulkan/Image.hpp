#pragma once

#include "ProjectThalia/Rendering/Vulkan/DeviceObject.hpp"
#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class Image : DeviceObject
	{
		public:
			Image(const Device* device, const char* pixels, vk::DeviceSize pixelsSizeInBytes);

		private:
			vk::Image _vkImage;
	};
}
