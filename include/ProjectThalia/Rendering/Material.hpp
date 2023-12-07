#pragma once

#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/Rendering/Vulkan/Pipeline.hpp"
#include "ProjectThalia/Rendering/Texture2D.hpp"

#include "Shader.hpp"

namespace ProjectThalia::Rendering
{
	class Material
	{
		public:
			Material() = default;

			explicit Material(Shader* shader);

			~Material();

			void SetTexture(size_t index, const Texture2D& texture);

			void Update();

			[[nodiscard]] Shader* GetShader() const;

			[[nodiscard]] Vulkan::DescriptorSetManager::DescriptorSetAllocation&       GetDescriptorSetAllocation();
			[[nodiscard]] const Vulkan::DescriptorSetManager::DescriptorSetAllocation& GetDescriptorSetAllocation() const;

		private:
			Shader* _shader = nullptr;

			std::vector<vk::WriteDescriptorSet> _updateImageWriteDescriptorSets;

			Vulkan::DescriptorSetManager::DescriptorSetAllocation _descriptorSetAllocation;

			template<typename T>
			T* const GetUniformBufferObjectAs(uint32_t uniformBufferIndex = 0)
			{
				return _descriptorSetAllocation.ShaderBuffers[uniformBufferIndex].GetMappedData<T>();
			}
	};
}