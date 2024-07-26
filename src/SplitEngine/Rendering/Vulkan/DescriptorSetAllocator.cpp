#include "SplitEngine/Rendering/Vulkan/DescriptorSetAllocator.hpp"
#include "SplitEngine/Rendering/Vulkan/Buffer.hpp"
#include "SplitEngine/Rendering/Vulkan/BufferFactory.hpp"
#include "SplitEngine/Rendering/Vulkan/Device.hpp"
#include "SplitEngine/Rendering/Vulkan/Utility.hpp"

#include <numeric>
#include <utility>
#include <vector>

#include "SplitEngine/Rendering/Vulkan/Instance.hpp"
#include "SplitEngine/Rendering/Vulkan/PhysicalDevice.hpp"

namespace SplitEngine::Rendering::Vulkan
{
	std::unordered_map<std::string, Descriptor> DescriptorSetAllocator::_sharedDescriptors = std::unordered_map<std::string, Descriptor>();

	uint32_t DescriptorSetAllocator::_descriptorIdCounter = 0;

	DescriptorSetAllocator::DescriptorSetAllocator(Device* device, CreateInfo& descriptorSetInfo, uint32_t maxSetsPerPool) :
		DeviceObject(device),
		_descriptorPoolSizes(std::move(descriptorSetInfo.DescriptorPoolSizes)),
		_writeDescriptorSets(std::move(descriptorSetInfo.WriteDescriptorSets)),
		_descriptorCreateInfo(std::move(descriptorSetInfo.DescriptorCreateInfos)),
		_maxSetsPerPool(maxSetsPerPool * Device::MAX_FRAMES_IN_FLIGHT)
	{
		const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo({}, descriptorSetInfo.DescriptorLayoutBindings);
		_descriptorSetLayout                                                  = GetDevice()->GetVkDevice().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		for (const auto& binding: descriptorSetInfo.Bindings) { _bindings.push_back(binding); }

		_descriptorSetLayouts = std::vector<vk::DescriptorSetLayout>(Device::MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);

		AllocateNewDescriptorPool();
	}

	DescriptorSetAllocator::Allocation DescriptorSetAllocator::AllocateDescriptorSet()
	{
		// Find pool with available space
		DescriptorPoolInstance* validDescriptorPoolInstance;
		uint32_t                descriptorPoolIndex = -1;
		for (uint32_t i = 0; i < _descriptorPoolInstances.size(); i++)
		{
			if (_descriptorPoolInstances[i].Available.GetSize() >= Device::MAX_FRAMES_IN_FLIGHT)
			{
				validDescriptorPoolInstance = &_descriptorPoolInstances[i];
				descriptorPoolIndex         = i;
				break;
			}
		}

		// Allocate new pool if existing are full
		if (descriptorPoolIndex == -1)
		{
			AllocateNewDescriptorPool();
			validDescriptorPoolInstance = &_descriptorPoolInstances.back();
			descriptorPoolIndex         = _descriptorPoolInstances.size() - 1;
		}


		// Allocate descriptor set
		std::vector<vk::DescriptorSet> descriptorSets            = std::vector<vk::DescriptorSet>(Device::MAX_FRAMES_IN_FLIGHT);
		vk::DescriptorSetAllocateInfo  descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo(validDescriptorPoolInstance->DescriptorPool,
		                                                                                         Device::MAX_FRAMES_IN_FLIGHT,
		                                                                                         _descriptorSetLayouts.data());

		GetDevice()->GetVkDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, descriptorSets.data());

		std::vector<DescriptorPoolAllocation> poolAllocations;
		for (int i = 0; i < Device::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			uint32_t insertionIndex                                     = validDescriptorPoolInstance->Available.Pop();
			validDescriptorPoolInstance->DescriptorSets[insertionIndex] = descriptorSets[i];
			poolAllocations.emplace_back(descriptorPoolIndex, insertionIndex);
		}

		Allocation descriptorSetAllocation;

		int numUniqueDescriptors = 0;
		for (DescriptorCreateInfo createInfo: _descriptorCreateInfo) { if (!createInfo.Shared) { numUniqueDescriptors++; } }

		// Create allocation object
		descriptorSetAllocation.DescriptorSets = InFlightResource<vk::DescriptorSet>(GetDevice()->GetCurrentFramePtr(), std::move(descriptorSets));

		descriptorSetAllocation._uniqueDescriptors.reserve(numUniqueDescriptors);

		// Create descriptor resources
		for (int i = 0; i < _writeDescriptorSets.size(); ++i)
		{
			Descriptor                           descriptor{};
			uint32_t                             bindingPoint         = _bindings[i];
			std::vector<vk::WriteDescriptorSet>& writeDescriptorSets  = _writeDescriptorSets[i];
			vk::WriteDescriptorSet&              lastWriteDescriptor  = writeDescriptorSets.back();
			DescriptorCreateInfo&                descriptorCreateInfo = _descriptorCreateInfo[i];

			descriptor.SingleInstance = descriptorCreateInfo.SingleInstance;

			// Skip if descriptor is shared and already exists
			if (_sharedDescriptors.contains(descriptorCreateInfo.Name))
			{
				descriptorSetAllocation.DescriptorEntries.emplace_back(bindingPoint, &_sharedDescriptors[descriptorCreateInfo.Name]);
				descriptorSetAllocation.WriteDescriptorSets.push_back(_sharedDescriptors[descriptorCreateInfo.Name].WriteDescriptorSets);

				for (int fifIndex = 0; fifIndex < Device::MAX_FRAMES_IN_FLIGHT; ++fifIndex)
				{
					vk::WriteDescriptorSet& writeDescriptor = descriptorSetAllocation.WriteDescriptorSets.back().GetDataVector()[fifIndex];
					writeDescriptor.dstSet                  = descriptorSetAllocation.DescriptorSets[fifIndex];
					writeDescriptor.dstBinding              = bindingPoint;
				}
			}
			else
			{
				// Create descriptor
				descriptor.WriteDescriptorSets = GetDevice()->CreateInFlightResource<vk::WriteDescriptorSet>();
				switch (lastWriteDescriptor.descriptorType)
				{
					case vk::DescriptorType::eUniformBuffer:
					case vk::DescriptorType::eStorageBuffer:
					{
						descriptor.Type = Descriptor::Type::Buffer;

						vk::Flags<vk::BufferUsageFlagBits> usage = lastWriteDescriptor.descriptorType == vk::DescriptorType::eUniformBuffer
							                                           ? vk::BufferUsageFlagBits::eUniformBuffer
							                                           : vk::BufferUsageFlagBits::eStorageBuffer;

						if (descriptorCreateInfo.TransferSrc) { usage |= vk::BufferUsageFlagBits::eTransferSrc; }

						if (descriptorCreateInfo.TransferDst) { usage |= vk::BufferUsageFlagBits::eTransferDst; }

						uint32_t                                             numSubBuffers = descriptorCreateInfo.SingleInstance ? 1 : Device::MAX_FRAMES_IN_FLIGHT;
						vk::Flags<Allocator::MemoryAllocationCreateFlagBits> flags{};
						vk::Flags<Allocator::MemoryPropertyFlagBits>         requiredFlags{};

						// Always try to presistent map and be coherant
						if (!descriptorCreateInfo.DeviceLocal)
						{
							flags |= Allocator::PersistentMap;
							requiredFlags |= Allocator::HostVisible;
							if (!descriptorCreateInfo.NoCoherant) { requiredFlags |= Allocator::HostCoherant; }
						}

						if (descriptorCreateInfo.DeviceLocalHostVisible)
						{
							requiredFlags |= Allocator::LocalDevice;
							requiredFlags |= Allocator::HostVisible;
							flags |= Allocator::WriteSequentially;
						}
						else if (descriptorCreateInfo.DeviceLocal) { requiredFlags |= Allocator::LocalDevice; }
						else
						{
							if (descriptorCreateInfo.Cached)
							{
								requiredFlags |= Allocator::HostCached;
								flags |= Allocator::RandomAccess;
							}
							else { flags |= Allocator::WriteSequentially; }
						}

						Allocator::MemoryAllocationCreateInfo allocationCreateInfo = { Allocator::Auto, flags, requiredFlags };

						if (!descriptorCreateInfo.NoAllocation)
						{
							descriptor.Buffer = std::move(Buffer(GetDevice(),
							                                     usage,
							                                     vk::SharingMode::eExclusive,
							                                     allocationCreateInfo,
							                                     numSubBuffers,
							                                     lastWriteDescriptor.pBufferInfo->range));
						}

						descriptor.BufferInfos = GetDevice()->CreateInFlightResource<vk::DescriptorBufferInfo>(descriptorCreateInfo.SingleInstance);
						descriptor.BufferPtrs  = GetDevice()->CreateInFlightResource<std::byte*>(descriptorCreateInfo.SingleInstance, nullptr);

						size_t offset   = 0;
						int    fifIndex = 0;
						for (int j = 0; j < Device::MAX_FRAMES_IN_FLIGHT; ++j)
						{
							descriptor.BufferInfos[fifIndex] = vk::DescriptorBufferInfo(*writeDescriptorSets[j].pBufferInfo);

							if (!descriptorCreateInfo.NoAllocation)
							{
								descriptor.BufferInfos[fifIndex].buffer = descriptor.Buffer.GetVkBuffer();
								if (!descriptorCreateInfo.DeviceLocal) { descriptor.BufferPtrs[fifIndex] = descriptor.Buffer.GetMappedData<std::byte>() + offset; }
							}
							else { descriptor.BufferPtrs[fifIndex] = nullptr; }

							descriptor.WriteDescriptorSets[j] = writeDescriptorSets[j];

							descriptor.WriteDescriptorSets[j].dstSet      = descriptorSetAllocation.DescriptorSets[j];
							descriptor.WriteDescriptorSets[j].pBufferInfo = &descriptor.BufferInfos[fifIndex];

							if (!descriptorCreateInfo.SingleInstance)
							{
								offset += lastWriteDescriptor.pBufferInfo->range;
								fifIndex++;
							}
						}
						break;
					}
					case vk::DescriptorType::eCombinedImageSampler:
					{
						descriptor.Type = Descriptor::Type::ImageSampler;

						descriptor.ImageInfos = GetDevice()->CreateInFlightResource<std::vector<vk::DescriptorImageInfo>>(descriptorCreateInfo.SingleInstance,
							std::vector<vk::DescriptorImageInfo>());

						int fifIndex = 0;
						for (int j = 0; j < Device::MAX_FRAMES_IN_FLIGHT; ++j)
						{
							if (!descriptorCreateInfo.SingleInstance || (descriptorCreateInfo.SingleInstance && j == 0))
							{
								for (int imageIndex = 0; imageIndex < lastWriteDescriptor.descriptorCount; ++imageIndex)
								{
									descriptor.ImageInfos[fifIndex].emplace_back(*GetDevice()->GetPhysicalDevice().GetInstance().GetDefaultSampler(),
									                                             GetDevice()->GetPhysicalDevice().GetInstance().GetDefaultImage().GetView(),
									                                             GetDevice()->GetPhysicalDevice().GetInstance().GetDefaultImage().GetLayout());
								}
							}

							descriptor.WriteDescriptorSets[j]            = writeDescriptorSets[j];
							descriptor.WriteDescriptorSets[j].dstSet     = descriptorSetAllocation.DescriptorSets[j];
							descriptor.WriteDescriptorSets[j].pImageInfo = descriptor.ImageInfos[fifIndex].data();

							if (!descriptorCreateInfo.SingleInstance) { fifIndex++; }
						}

						break;
					}
				}

				if (descriptorCreateInfo.Shared)
				{
					descriptor.SharedID                           = _descriptorIdCounter++;
					_sharedDescriptors[descriptorCreateInfo.Name] = std::move(descriptor);
					descriptorSetAllocation.DescriptorEntries.emplace_back(bindingPoint, &_sharedDescriptors[descriptorCreateInfo.Name]);
					descriptorSetAllocation.WriteDescriptorSets.push_back(_sharedDescriptors[descriptorCreateInfo.Name].WriteDescriptorSets);
				}
				else
				{
					descriptorSetAllocation._uniqueDescriptors.push_back(std::move(descriptor));
					descriptorSetAllocation.DescriptorEntries.emplace_back(bindingPoint, &descriptorSetAllocation._uniqueDescriptors.back());
					descriptorSetAllocation.WriteDescriptorSets.push_back(descriptorSetAllocation._uniqueDescriptors.back().WriteDescriptorSets);
				}
			}

			if (!descriptorCreateInfo.NoAllocation)
			{
				GetDevice()->GetVkDevice().updateDescriptorSets(Device::MAX_FRAMES_IN_FLIGHT, descriptorSetAllocation.WriteDescriptorSets.back().GetDataPtr(), 0, nullptr);
			}
			descriptorSetAllocation.SparseDescriptorLookup[bindingPoint] = descriptorSetAllocation.DescriptorEntries.size() - 1;
		}

		descriptorSetAllocation._descriptorPoolAllocations = std::move(poolAllocations);

		return descriptorSetAllocation;
	}

	void DescriptorSetAllocator::AllocateNewDescriptorPool()
	{
		const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		                                                                                           _maxSetsPerPool,
		                                                                                           _descriptorPoolSizes);

		AvailableStack<uint32_t> availableStack = AvailableStack<uint32_t>(_maxSetsPerPool);

		// Fill from 0 to size - 1
		std::iota(availableStack.begin(), availableStack.end(), 0);

		DescriptorPoolInstance descriptorPoolInstance = {
			GetDevice()->GetVkDevice().createDescriptorPool(descriptorPoolCreateInfo),
			std::vector<vk::DescriptorSet>(_maxSetsPerPool, VK_NULL_HANDLE),
			std::move(availableStack)
		};

		_descriptorPoolInstances.push_back(std::move(descriptorPoolInstance));
	}

	void DescriptorSetAllocator::Destroy()
	{
		Utility::DeleteDeviceHandle(GetDevice(), _descriptorSetLayout);
		for (const DescriptorPoolInstance& descriptorPoolInstance: _descriptorPoolInstances) { Utility::DeleteDeviceHandle(GetDevice(), descriptorPoolInstance.DescriptorPool); }

		for (std::vector<vk::WriteDescriptorSet>& writeDescriptorSets: _writeDescriptorSets)
		{
			for (const vk::WriteDescriptorSet& writeDescriptorSet: writeDescriptorSets) { delete writeDescriptorSet.pBufferInfo; }
		}
	}

	void DescriptorSetAllocator::DeallocateDescriptorSet(DescriptorSetAllocator::Allocation& descriptorSetAllocation)
	{
		if (descriptorSetAllocation.DescriptorSets.IsValid())
		{
			for (const auto& poolAllocation: descriptorSetAllocation._descriptorPoolAllocations)
			{
				DescriptorPoolInstance& descriptorPoolInstance = _descriptorPoolInstances[poolAllocation.DescriptorPoolIndex];

				GetDevice()->GetVkDevice().freeDescriptorSets(descriptorPoolInstance.DescriptorPool, 1, &descriptorPoolInstance.DescriptorSets[poolAllocation.DescriptorSetIndex]);

				for (int i = 0; i < Device::MAX_FRAMES_IN_FLIGHT; ++i)
				{
					descriptorPoolInstance.DescriptorSets[poolAllocation.DescriptorSetIndex] = VK_NULL_HANDLE;
					descriptorPoolInstance.Available.Push(poolAllocation.DescriptorSetIndex);
				}
			}

			for (auto& descriptor: descriptorSetAllocation._uniqueDescriptors) { if (descriptor.Type == Descriptor::Type::Buffer) { descriptor.Buffer.Destroy(); } }
		}
	}

	const vk::DescriptorSetLayout& DescriptorSetAllocator::GetDescriptorSetLayout() const { return _descriptorSetLayout; }
}
