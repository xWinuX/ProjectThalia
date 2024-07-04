#pragma once

#include "Buffer.hpp"
#include "DeviceObject.hpp"

#include "InFlightResource.hpp"
#include "SplitEngine/DataStructures.hpp"
#include "vulkan/vulkan.hpp"

#include <unordered_set>
#include <vector>

#include "Descriptor.hpp"

namespace SplitEngine::Rendering
{
	class Shader;

	namespace Vulkan
	{
		class Device;

		class Instance;

		class Buffer;

		class DescriptorSetAllocator final : public DeviceObject
		{
			friend Shader;
			friend Instance;

			public:
				struct DescriptorCreateInfo
				{
					public:
						std::string Name;
						bool        SingleInstance         = false;
						bool        DeviceLocal            = false;
						bool        DeviceLocalHostVisible = false;
						bool        Cached                 = false;
						bool        Shared                 = false;
						bool        NoAllocation           = false;
						bool        NoCoherant             = false;
				};

				struct CreateInfo
				{
					public:
						std::unordered_set<uint32_t>                     Bindings                 = std::unordered_set<uint32_t>();
						std::vector<vk::DescriptorSetLayoutBinding>      DescriptorLayoutBindings = std::vector<vk::DescriptorSetLayoutBinding>(0);
						std::vector<vk::DescriptorPoolSize>              DescriptorPoolSizes      = std::vector<vk::DescriptorPoolSize>(0);
						std::vector<std::vector<vk::WriteDescriptorSet>> WriteDescriptorSets      = std::vector<std::vector<vk::WriteDescriptorSet>>(0);
						std::vector<DescriptorCreateInfo>                DescriptorCreateInfos    = std::vector<DescriptorCreateInfo>(0);
				};

				struct DescriptorPoolInstance
				{
					public:
						vk::DescriptorPool             DescriptorPool;
						std::vector<vk::DescriptorSet> DescriptorSets;
						AvailableStack<uint32_t>       Available;
				};

				struct DescriptorPoolAllocation
				{
					uint32_t DescriptorPoolIndex;
					uint32_t DescriptorSetIndex;
				};

				struct Allocation
				{
					friend DescriptorSetAllocator;

					public:
						struct DescriptorEntry
						{
							uint32_t    BindingPoint = -1;
							Descriptor* Descriptor   = nullptr;
						};

						InFlightResource<vk::DescriptorSet>                   DescriptorSets{};
						std::vector<DescriptorEntry>                          DescriptorEntries;
						std::vector<InFlightResource<vk::WriteDescriptorSet>> WriteDescriptorSets;
						std::vector<uint32_t>                                 SparseDescriptorLookup = std::vector<uint32_t>(12, -1);

					private:
						std::vector<Descriptor>               _uniqueDescriptors;
						std::vector<DescriptorPoolAllocation> _descriptorPoolAllocations;
				};

			public:
				DescriptorSetAllocator() = default;
				DescriptorSetAllocator(Device* device, CreateInfo& descriptorSetInfo, uint32_t maxSetsPerPool);

				void Destroy() override;

				Allocation AllocateDescriptorSet();

				void DeallocateDescriptorSet(Allocation& descriptorSetAllocation);

				[[nodiscard]] const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

			private:
				std::vector<uint32_t>                            _bindings;
				vk::DescriptorSetLayout                          _descriptorSetLayout;
				std::vector<vk::DescriptorSetLayout>             _descriptorSetLayouts;
				std::vector<DescriptorPoolInstance>              _descriptorPoolInstances;
				std::vector<vk::DescriptorPoolSize>              _descriptorPoolSizes;
				std::vector<std::vector<vk::WriteDescriptorSet>> _writeDescriptorSets;
				std::vector<DescriptorCreateInfo>                _descriptorCreateInfo;
				uint32_t                                         _maxSetsPerPool = 10;

				static uint32_t                                    _descriptorIdCounter;
				static std::unordered_map<std::string, Descriptor> _sharedDescriptors;

				void AllocateNewDescriptorPool();
		};
	}
}
