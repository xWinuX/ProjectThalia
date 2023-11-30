#include "ProjectThalia/Rendering/Vulkan/DescriptorSetManager.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/Rendering/Vulkan/Device.hpp"
#include "ProjectThalia/Rendering/Vulkan/Utility.hpp"

#include <numeric>
#include <utility>
#include <vector>

namespace ProjectThalia::Rendering::Vulkan
{
	DescriptorSetManager::DescriptorSetManager(Device*                                     device,
											   std::vector<vk::DescriptorSetLayoutBinding> descriptorLayoutBindings,
											   std::vector<vk::DescriptorPoolSize>         descriptorPoolSizes,
											   std::vector<vk::WriteDescriptorSet>         writeDescriptorSets,
											   uint32_t                                    maxSetsPerPool) :
		DeviceObject(device),
		_descriptorPoolSizes(std::move(descriptorPoolSizes)),
		_writeDescriptorSets(std::move(writeDescriptorSets)),
		_maxSetsPerPool(maxSetsPerPool)
	{
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo({}, descriptorLayoutBindings);
		_descriptorSetLayout                                            = GetDevice()->GetVkDevice().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		AllocateNewDescriptorPool();
	}

	DescriptorSetManager::DescriptorSetAllocation DescriptorSetManager::AllocateDescriptorSet()
	{
		// Find pool with available space
		DescriptorPoolInstance validDescriptorPoolInstance;
		uint32_t               descriptorPoolIndex = -1;
		for (uint32_t i = 0; i < _descriptorPoolInstances.size(); ++i)
		{
			if (!_descriptorPoolInstances[i].Available.IsEmpty())
			{
				validDescriptorPoolInstance = _descriptorPoolInstances[i];
				descriptorPoolIndex         = i;
				break;
			}
		}

		// Allocate new pool if existing are full
		if (descriptorPoolIndex == -1)
		{
			AllocateNewDescriptorPool();
			validDescriptorPoolInstance = _descriptorPoolInstances.back();
			descriptorPoolIndex         = _descriptorPoolInstances.size() - 1;
		}

		// Allocate descriptor set
		vk::DescriptorSet             descriptorSet;
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo(validDescriptorPoolInstance.DescriptorPool,
																								_descriptorSetLayout);
		GetDevice()->GetVkDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, &descriptorSet);

		uint32_t insertionIndex                                    = validDescriptorPoolInstance.Available.Pop();
		validDescriptorPoolInstance.DescriptorSets[insertionIndex] = descriptorSet;

		// Create allocation object
		DescriptorSetAllocation descriptorSetAllocation;
		descriptorSetAllocation.DescriptorSet        = validDescriptorPoolInstance.DescriptorSets[insertionIndex];
		descriptorSetAllocation._descriptorPoolIndex = descriptorPoolIndex;
		descriptorSetAllocation._descriptorSetIndex  = insertionIndex;

		return descriptorSetAllocation;
	}

	void DescriptorSetManager::AllocateNewDescriptorPool()
	{
		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo({}, _maxSetsPerPool, _descriptorPoolSizes);

		DescriptorPoolInstance descriptorPoolInstance = {GetDevice()->GetVkDevice().createDescriptorPool(descriptorPoolCreateInfo),
														 std::vector<vk::DescriptorSet>(_maxSetsPerPool, VK_NULL_HANDLE),
														 AvailableStack(_maxSetsPerPool)};
		_descriptorPoolInstances.push_back(std::move(descriptorPoolInstance));
	}

	void DescriptorSetManager::Destroy()
	{
		Utility::DeleteDeviceHandle(GetDevice(), _descriptorSetLayout);
		for (const DescriptorPoolInstance& descriptorPoolInstance : _descriptorPoolInstances)
		{
			for (const vk::DescriptorSet& descriptorSet : descriptorPoolInstance.DescriptorSets)
			{
				if (descriptorSet == VK_NULL_HANDLE) { continue; }
				GetDevice()->GetVkDevice().freeDescriptorSets(descriptorPoolInstance.DescriptorPool, descriptorSet);
			}
			Utility::DeleteDeviceHandle(GetDevice(), descriptorPoolInstance.DescriptorPool);
		}

		for (const vk::WriteDescriptorSet& writeDescriptorSet : _writeDescriptorSets)
		{
			delete writeDescriptorSet.pBufferInfo;
			delete writeDescriptorSet.pImageInfo;
		}
	}

	void DescriptorSetManager::DeallocateDescriptorSet(DescriptorSetManager::DescriptorSetAllocation& descriptorSetAllocation)
	{
		DescriptorPoolInstance& descriptorPoolInstance = _descriptorPoolInstances[descriptorSetAllocation._descriptorPoolIndex];
		GetDevice()->GetVkDevice().freeDescriptorSets(descriptorPoolInstance.DescriptorPool,
													  descriptorPoolInstance.DescriptorSets[descriptorSetAllocation._descriptorSetIndex]);
		descriptorPoolInstance.DescriptorSets[descriptorSetAllocation._descriptorSetIndex] = VK_NULL_HANDLE;
		descriptorPoolInstance.Available.Push(descriptorSetAllocation._descriptorSetIndex);
	}

	const vk::DescriptorSetLayout& DescriptorSetManager::GetDescriptorSetLayout() const { return _descriptorSetLayout; }

	DescriptorSetManager::AvailableStack::AvailableStack(uint32_t size) :
		_vector(std::vector<uint32_t>(size, 0)),
		_cursor(size - 1)
	{
		// Fill vector with numbers ascending from 0 to vector size - 1
		std::iota(std::begin(_vector), std::end(_vector), 0);
	}

	uint32_t DescriptorSetManager::AvailableStack::Pop() { return _vector[_cursor--]; }

	void DescriptorSetManager::AvailableStack::Push(uint32_t value) { _vector[++_cursor] = value; }

	bool DescriptorSetManager::AvailableStack::IsEmpty() const { return _cursor == 0; }
}