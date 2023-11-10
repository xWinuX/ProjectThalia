#pragma once

#include "DeviceObject.hpp"
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class DescriptorPool : public DeviceObject
	{
		public:
			DescriptorPool() = default;
			DescriptorPool(Device* device, uint32_t maxSets, const std::vector<vk::DescriptorPoolSize>& descriptorPoolConfiguration);

			
		private:
			vk::DescriptorPool                               _vkDescriptorPool;
			uint32_t                                         _availableSets = 0;
			std::unordered_map<vk::DescriptorType, uint32_t> _availableTypes;
	};
}
