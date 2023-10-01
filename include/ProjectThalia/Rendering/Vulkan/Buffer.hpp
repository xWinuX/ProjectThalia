#pragma once

#include "vulkan/vulkan.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	class Buffer
	{
		public:
			Buffer() = default;
			Buffer(const vk::Device&                         device,
				   const vk::PhysicalDeviceMemoryProperties& memoryProperties,
				   const char*                               data,
				   vk::DeviceSize                            size,
				   vk::BufferUsageFlagBits                   usage,
				   vk::SharingMode                           sharingMode);

			[[nodiscard]] const vk::Buffer& GetVkBuffer() const;

			void Destroy(vk::Device device);

		private:
			vk::Buffer       _vkBuffer;
			vk::DeviceMemory _memory;
	};
}