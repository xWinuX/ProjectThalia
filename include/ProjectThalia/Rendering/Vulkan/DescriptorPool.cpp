#include "DescriptorPool.hpp"
#include "Device.hpp"

namespace ProjectThalia::Rendering::Vulkan
{
	DescriptorPool::DescriptorPool(Device* device, uint32_t maxSets, const std::vector<vk::DescriptorPoolSize>& descriptorPoolConfiguration) :
		DeviceObject(device)
	{
		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo({}, maxSets, descriptorPoolConfiguration);
		_vkDescriptorPool                                     = GetDevice()->GetVkDevice().createDescriptorPool(descriptorPoolCreateInfo);

		for (const vk::DescriptorPoolSize& item : descriptorPoolConfiguration) { _availableTypes[item.type] = item.descriptorCount; }
	}
}
