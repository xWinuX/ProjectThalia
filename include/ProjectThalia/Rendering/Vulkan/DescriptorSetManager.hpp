#pragma once

#include "DeviceObject.hpp"
#include "vulkan/vulkan.hpp"

#include "stack"

namespace ProjectThalia::Rendering::Vulkan
{
	class Device;

	class DescriptorSetManager final : DeviceObject
	{
		public:
			class AvailableStack
			{
				public:
					AvailableStack() = default;
					explicit AvailableStack(uint32_t size);

					uint32_t Pop();
					void     Push(uint32_t value);

					[[nodiscard]] bool IsEmpty() const;

				private:
					std::vector<uint32_t> _vector;
					uint32_t              _cursor = -1;
			};

			struct DescriptorPoolInstance
			{
				public:
					vk::DescriptorPool             DescriptorPool;
					std::vector<vk::DescriptorSet> DescriptorSets;
					AvailableStack                 Available;
			};

			struct DescriptorSetAllocation
			{
					friend DescriptorSetManager;

				public:
					vk::DescriptorSet DescriptorSet = VK_NULL_HANDLE;
					

				private:
					uint32_t _descriptorPoolIndex = -1;
					uint32_t _descriptorSetIndex  = -1;
			};

		public:
			DescriptorSetManager() = default;
			DescriptorSetManager(Device*                                     device,
								 std::vector<vk::DescriptorSetLayoutBinding> descriptorLayoutBindings,
								 std::vector<vk::DescriptorPoolSize>         descriptorPoolSizes,
								 std::vector<vk::WriteDescriptorSet>         writeDescriptorSets,
								 uint32_t                                    maxSetsPerPool);

			void Destroy() override;

			DescriptorSetAllocation AllocateDescriptorSet();

			void DeallocateDescriptorSet(DescriptorSetAllocation& descriptorSetAllocation);

			[[nodiscard]] const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

		private:
			vk::DescriptorSetLayout             _descriptorSetLayout;
			std::vector<DescriptorPoolInstance> _descriptorPoolInstances;
			std::vector<vk::DescriptorPoolSize> _descriptorPoolSizes;
			std::vector<vk::WriteDescriptorSet> _writeDescriptorSets;
			uint32_t                            _maxSetsPerPool;
			void                                AllocateNewDescriptorPool();
	};
}