#pragma once

#include "SplitEngine/AssetDatabase.hpp"
#include "SplitEngine/Rendering/Vulkan/InFlightResource.hpp"
#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"

#include <string>

namespace SplitEngine::Rendering
{

	class Texture2D;

	class Shader
	{
		public:
			struct CreateInfo
			{
				public:
					std::string ShaderPath;
			};

			class Properties
			{
					friend Shader;
					friend class Material;

				public:
					Properties() = default;
					Properties(Shader* shader, Vulkan::DescriptorSetAllocator::Allocation* descriptorSetAllocation);

					template<typename T>
					Vulkan::InFlightResource<std::byte*> GetBufferRaw(uint32_t bindingPoint)
					{
						return _descriptorSetAllocation->ShaderBufferPtrs[_descriptorSetAllocation->SparseShaderBufferLookup[bindingPoint]];
					}

					template<typename T>
					T* GetBuffer(uint32_t bindingPoint)
					{
						return reinterpret_cast<T*>(_descriptorSetAllocation->ShaderBufferPtrs[_descriptorSetAllocation->SparseShaderBufferLookup[bindingPoint]].Get());
					}

					template<typename T>
					T** GetStorableBuffer(uint32_t bindingPoint)
					{
						return reinterpret_cast<T**>(&_shaderBufferPtrs[_descriptorSetAllocation->SparseShaderBufferLookup[bindingPoint]]);
					}

					void SetTexture(uint32_t bindingPoint, const Texture2D& texture);
					void SetTextures(uint32_t bindingPoint, size_t offset, std::vector<std::unique_ptr<Texture2D>>& textures);
					void SetTextures(uint32_t bindingPoint, size_t offset, std::vector<AssetHandle<Texture2D>>& textures);

				private:
					Shader*                                           _shader                  = nullptr;
					Vulkan::DescriptorSetAllocator::Allocation*       _descriptorSetAllocation = nullptr;
					std::vector<Vulkan::InFlightResource<std::byte*>> _shaderBuffers {};
					std::vector<vk::WriteDescriptorSet>               _updateImageWriteDescriptorSets {};

					std::vector<std::byte*> _shaderBufferPtrs {};

					void SetWriteDescriptorSetDirty(uint32_t bindingPoint);

					void Update();
			};

		public:
			Shader() = default;
			explicit Shader(const CreateInfo& createInfo);

			~Shader();

			void BindGlobal(vk::CommandBuffer& commandBuffer);

			void Bind(vk::CommandBuffer& commandBuffer);

			static void UpdateGlobal();

			void Update();

			Properties&        GetProperties();
			static Properties& GetGlobalProperties();

			[[nodiscard]] Vulkan::Pipeline& GetPipeline();

		private:
			Vulkan::Device*  _device = nullptr;
			Vulkan::Pipeline _pipeline {};
			std::string      _shaderPath {};

			static Properties _globalProperties;
			static bool       _globalPropertiesDefined;

			Properties _shaderProperties {};
	};

}