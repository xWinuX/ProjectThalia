#pragma once

#include "DescriptorSetAllocator.hpp"
#include "DeviceObject.hpp"
#include "spirv_common.hpp"

#include <vulkan/vulkan.hpp>

namespace SplitEngine::Rendering::Vulkan
{
	class Device;

	class Pipeline final : public DeviceObject
	{
		public:
			friend class Instance;

			enum class ShaderType
			{
				Vertex   = VK_SHADER_STAGE_VERTEX_BIT,
				Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
				Compute  = VK_SHADER_STAGE_COMPUTE_BIT
			};


			struct ShaderInfo
			{
				public:
					std::string path;
					ShaderType  shaderStage;
			};

			Pipeline() = default;

			Pipeline(Device* device, const std::string& name, const std::vector<ShaderInfo>& shaderInfos);

			void Bind(const vk::CommandBuffer& commandBuffer) const;

			void BindDescriptorSets(const vk::CommandBuffer& commandBuffer,
			                        uint32_t                 descriptorSetCount,
			                        const vk::DescriptorSet* descriptorSets,
			                        uint32_t                 firstSet,
			                        uint32_t                 dynamicOffsetCount = 0,
			                        uint32_t*                dynamicOffsets     = nullptr) const;

			void BindDescriptorSets(const vk::CommandBuffer&            commandBuffer,
			                        DescriptorSetAllocator::Allocation* descriptorSetAllocation,
			                        uint32_t                            firstSet,
			                        uint32_t                            dynamicOffsetCount = 0,
			                        uint32_t*                           dynamicOffsets     = nullptr,
			                        uint32_t                            frameInFlight      = -1) const;
			void Destroy() override;

			[[nodiscard]] const vk::Pipeline&       GetVkPipeline() const;
			[[nodiscard]] const vk::PipelineLayout& GetLayout() const;


			[[nodiscard]] static DescriptorSetAllocator::Allocation& GetGlobalDescriptorSetAllocation();
			[[nodiscard]] DescriptorSetAllocator::Allocation&        GetPerPipelineDescriptorSetAllocation();

			[[nodiscard]] DescriptorSetAllocator::Allocation AllocatePerInstanceDescriptorSet();
			void                                             DeallocatePerInstanceDescriptorSet(DescriptorSetAllocator::Allocation& descriptorSetAllocation);

		private:
			vk::Pipeline          _vkPipeline;
			vk::PipelineLayout    _layout;
			vk::PipelineBindPoint _bindPoint;

			static DescriptorSetAllocator             _globalDescriptorManager;
			static DescriptorSetAllocator::Allocation _globalDescriptorSetAllocation;
			static bool                               _globalDescriptorsProcessed;

			DescriptorSetAllocator _perInstanceDescriptorSetManager;
			DescriptorSetAllocator _perPipelineDescriptorSetManager;

			std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts;

			DescriptorSetAllocator::Allocation _perPipelineDescriptorSetAllocation;

			std::vector<vk::ShaderModule> _shaderModules;

			[[nodiscard]] static vk::Format GetFormatFromType(const spirv_cross::SPIRType& type);
	};
}
