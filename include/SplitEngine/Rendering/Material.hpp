#pragma once

#include "SplitEngine/Rendering/Vulkan/Buffer.hpp"
#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"
#include "SplitEngine/Rendering/Texture2D.hpp"

#include "Shader.hpp"

namespace SplitEngine::Rendering
{
	class Material
	{
		public:
			Material() = default;

			explicit Material(Shader* shader);

			~Material();

			void SetTexture(size_t index, const Texture2D& texture);
			void SetTextures(size_t index, size_t offset, const std::vector<Texture2D*>& textures);

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

			void SetWriteDescriptorSetDirty(size_t index);
	};
}