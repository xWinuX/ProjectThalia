#include "SplitEngine/Rendering/Vulkan/DescriptorSetAllocator.hpp"
#include "SplitEngine/Rendering/Vulkan/Buffer.hpp"
#include "SplitEngine/Rendering/Vulkan/BufferFactory.hpp"
#include "SplitEngine/Rendering/Vulkan/Device.hpp"
#include "SplitEngine/Rendering/Vulkan/Utility.hpp"

#include <numeric>
#include <utility>
#include <vector>

namespace SplitEngine::Rendering::Vulkan
{
	DescriptorSetAllocator::DescriptorSetAllocator(Device* device, CreateInfo& descriptorSetInfo, uint32_t maxSetsPerPool) :
		DeviceObject(device),
		_descriptorPoolSizes(std::move(descriptorSetInfo.descriptorPoolSizes)),
		_writeDescriptorSets(std::move(descriptorSetInfo.writeDescriptorSets)),
		_bufferCreateInfo(std::move(descriptorSetInfo.bufferCreateInfos)),
		_maxSetsPerPool(maxSetsPerPool * Device::MAX_FRAMES_IN_FLIGHT)
	{
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo({}, descriptorSetInfo.descriptorLayoutBindings);
		_descriptorSetLayout                                            = GetDevice()->GetVkDevice().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		for (const auto& binding : descriptorSetInfo.bindings) { _bindings.push_back(binding); }

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

		// Create descriptor resources
		std::vector<std::vector<vk::DescriptorBufferInfo*>> bufferInfos = std::vector<std::vector<vk::DescriptorBufferInfo*>>();

		for (int i = 0; i < _writeDescriptorSets.size(); ++i)
		{
			uint32_t binding = _bindings[i];
			LOG("----- BINDING {0}", binding);
			std::vector<vk::WriteDescriptorSet>& writeDescriptorSets = _writeDescriptorSets[i];
			vk::WriteDescriptorSet&              lastWriteDescriptor = writeDescriptorSets.back();
			BufferCreateInfo&                    bufferCreateInfo    = _bufferCreateInfo[i];
			switch (lastWriteDescriptor.descriptorType)
			{
				case vk::DescriptorType::eUniformBuffer:
				case vk::DescriptorType::eStorageBuffer:
				{
					LOG("is buffer");
					vk::Flags<vk::BufferUsageFlagBits> usage = lastWriteDescriptor.descriptorType == vk::DescriptorType::eUniformBuffer
																	   ? vk::BufferUsageFlagBits::eUniformBuffer
																	   : vk::BufferUsageFlagBits::eStorageBuffer;

					uint32_t                                             numSubBuffers = bufferCreateInfo.SingleInstance ? 1 : Device::MAX_FRAMES_IN_FLIGHT;
					vk::Flags<Allocator::MemoryAllocationCreateFlagBits> flags {};
					vk::Flags<Allocator::MemoryPropertyFlagBits>         requiredFlags {};

					if (bufferCreateInfo.Cached) { flags |= Allocator::RandomAccess; }
					else { flags |= Allocator::WriteSequentially; }

					if (bufferCreateInfo.DeviceLocal) { requiredFlags |= Allocator::LocalDevice; }
					else { flags |= Allocator::PersistentMap; }

					if (bufferCreateInfo.Cached) { requiredFlags |= Allocator::HostCached; }

					Allocator::MemoryAllocationCreateInfo allocationCreateInfo = {Allocator::Auto, flags, requiredFlags};

					descriptorSetAllocation.ShaderBuffers.push_back(std::move(Buffer(GetDevice(),
																					 usage,
																					 vk::SharingMode::eExclusive,
																					 allocationCreateInfo,
																					 numSubBuffers,
																					 lastWriteDescriptor.pBufferInfo->range)));


					descriptorSetAllocation.SparseShaderBufferLookup[binding] = descriptorSetAllocation.ShaderBuffers.size() - 1;

					std::vector<std::byte*> bufferPtrs;
					size_t                  offset = 0;
					for (int j = 0; j < Device::MAX_FRAMES_IN_FLIGHT; ++j)
					{
						vk::WriteDescriptorSet& writeDescriptorSet = writeDescriptorSets[j];

						vk::DescriptorBufferInfo* bufferInfo = new vk::DescriptorBufferInfo(*writeDescriptorSet.pBufferInfo);
						Buffer&                   buffer     = descriptorSetAllocation.ShaderBuffers.back();

						bufferInfo->buffer = buffer.GetVkBuffer();
						delete writeDescriptorSet.pBufferInfo;
						writeDescriptorSet.dstSet      = descriptorSets[j];
						writeDescriptorSet.pBufferInfo = bufferInfo;

						bufferPtrs.push_back(buffer.GetMappedData<std::byte>() + offset);

						if (!bufferCreateInfo.SingleInstance) { offset += buffer.GetSizeInBytes(0); }
					}

					descriptorSetAllocation.ShaderBufferPtrs.emplace_back(GetDevice()->GetCurrentFramePtr(), std::move(bufferPtrs));

					break;
				}
				case vk::DescriptorType::eCombinedImageSampler:
				{
					LOG("is image sampler");
					descriptorSetAllocation.ImageInfos.emplace_back();
					for (int j = 0; j < lastWriteDescriptor.descriptorCount; ++j)
					{
						descriptorSetAllocation.ImageInfos.back().emplace_back(*GetDevice()->GetDefaultSampler(),
																			   GetDevice()->GetDefaultImage().GetView(),
																			   GetDevice()->GetDefaultImage().GetLayout());
					}

					std::vector<vk::WriteDescriptorSet> imageWriteDescriptorSets = std::vector<vk::WriteDescriptorSet>(Device::MAX_FRAMES_IN_FLIGHT);
					for (int j = 0; j < Device::MAX_FRAMES_IN_FLIGHT; ++j)
					{
						vk::WriteDescriptorSet& writeDescriptorSet = writeDescriptorSets[j];
						writeDescriptorSet.dstSet                  = descriptorSets[j];
						writeDescriptorSet.pImageInfo              = descriptorSetAllocation.ImageInfos.back().data();
						imageWriteDescriptorSets[j]                = writeDescriptorSet;
					}

					descriptorSetAllocation.SparseImageLookup[binding] = descriptorSetAllocation.ImageInfos.size() - 1;
					descriptorSetAllocation.ImageWriteDescriptorSets.emplace_back(GetDevice()->GetCurrentFramePtr(), std::move(imageWriteDescriptorSets));

					break;
				}
			}

			GetDevice()->GetVkDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}


		// Create allocation object
		descriptorSetAllocation.DescriptorSets = InFlightResource<vk::DescriptorSet>(GetDevice()->GetCurrentFramePtr(), std::move(descriptorSets));


		LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!! asdjkalsdjasd {0}", descriptorSetAllocation.ShaderBufferPtrs.size());

		descriptorSetAllocation._descriptorPoolAllocations = std::move(poolAllocations);

		return descriptorSetAllocation;
	}

	void DescriptorSetAllocator::AllocateNewDescriptorPool()
	{

		for (const auto& poolSize : _descriptorPoolSizes)
		{
			LOG("pool sizes type {0}", (int) poolSize.type);
			LOG("pool sizes count {0}", poolSize.descriptorCount);
		}

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
																							 _maxSetsPerPool,
																							 _descriptorPoolSizes);

		AvailableStack<uint32_t> availableStack = AvailableStack<uint32_t>(_maxSetsPerPool);

		// Fill from 0 to size - 1
		std::iota(availableStack.begin(), availableStack.end(), 0);

		DescriptorPoolInstance descriptorPoolInstance = {GetDevice()->GetVkDevice().createDescriptorPool(descriptorPoolCreateInfo),
														 std::vector<vk::DescriptorSet>(_maxSetsPerPool, VK_NULL_HANDLE),
														 std::move(availableStack)};

		_descriptorPoolInstances.push_back(std::move(descriptorPoolInstance));
	}

	void DescriptorSetAllocator::Destroy()
	{
		Utility::DeleteDeviceHandle(GetDevice(), _descriptorSetLayout);
		for (const DescriptorPoolInstance& descriptorPoolInstance : _descriptorPoolInstances)
		{
			Utility::DeleteDeviceHandle(GetDevice(), descriptorPoolInstance.DescriptorPool);
		}

		for (std::vector<vk::WriteDescriptorSet>& writeDescriptorSets : _writeDescriptorSets)
		{
			for (const vk::WriteDescriptorSet& writeDescriptorSet : writeDescriptorSets) { delete writeDescriptorSet.pBufferInfo; }
		}
	}

	void DescriptorSetAllocator::DeallocateDescriptorSet(DescriptorSetAllocator::Allocation& descriptorSetAllocation)
	{
		if (descriptorSetAllocation.DescriptorSets.IsValid())
		{
			for (const auto& poolAllocation : descriptorSetAllocation._descriptorPoolAllocations)
			{
				DescriptorPoolInstance& descriptorPoolInstance = _descriptorPoolInstances[poolAllocation.DescriptorPoolIndex];

				LOG("dealloc pool index {0}", poolAllocation.DescriptorPoolIndex);
				LOG("dealloc set index {0}", poolAllocation.DescriptorSetIndex);

				GetDevice()->GetVkDevice().freeDescriptorSets(descriptorPoolInstance.DescriptorPool,
															  1,
															  &descriptorPoolInstance.DescriptorSets[poolAllocation.DescriptorSetIndex]);

				for (int i = 0; i < Device::MAX_FRAMES_IN_FLIGHT; ++i)
				{
					descriptorPoolInstance.DescriptorSets[poolAllocation.DescriptorSetIndex] = VK_NULL_HANDLE;
					descriptorPoolInstance.Available.Push(poolAllocation.DescriptorSetIndex);
				}
			}

			LOG("Deallocate buffer");
			for (auto& buffer : descriptorSetAllocation.ShaderBuffers) { buffer.Destroy(); }
			LOG("after Deallocate buffer");
		}
	}

	const vk::DescriptorSetLayout& DescriptorSetAllocator::GetDescriptorSetLayout() const { return _descriptorSetLayout; }
}