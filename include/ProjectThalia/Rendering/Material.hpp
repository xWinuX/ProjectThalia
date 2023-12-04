#pragma once

#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/Rendering/Vulkan/Pipeline.hpp"

#include "Shader.hpp"

namespace ProjectThalia::Rendering
{
	class Material
	{
		public:
			Material() = default;

			explicit Material(Shader* shader);

			~Material();

			[[nodiscard]] Shader* GetShader() const;

			[[nodiscard]] Vulkan::DescriptorSetManager::DescriptorSetAllocation&       GetDescriptorSetAllocation();
			[[nodiscard]] const Vulkan::DescriptorSetManager::DescriptorSetAllocation& GetDescriptorSetAllocation() const;

		private:
			Shader* _shader = nullptr;

			Vulkan::DescriptorSetManager::DescriptorSetAllocation _descriptorSetAllocation;

			template<typename T>
			T* const GetUniformBufferObjectAs(uint32_t uniformBufferIndex = 0)
			{
				return _descriptorSetAllocation.ShaderBuffers[uniformBufferIndex].GetMappedData<T>();
			}
	};
}