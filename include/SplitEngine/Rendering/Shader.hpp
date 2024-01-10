#pragma once

#include "SplitEngine/AssetDatabase.hpp"
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
					T* GetBuffer(uint32_t bindingPoint)
					{
						return _descriptorSetAllocation->ShaderBuffers[bindingPoint].GetMappedData<T>();
					}

					void SetTexture(uint32_t bindingPoint, const Texture2D& texture);
					void SetTextures(uint32_t index, size_t offset, std::vector<std::unique_ptr<Texture2D>>& textures);
					void SetTextures(uint32_t index, size_t offset, std::vector<AssetHandle<Texture2D>>& textures);

				private:
					Shader*                                                  _shader                  = nullptr;
					Vulkan::DescriptorSetAllocator::Allocation* _descriptorSetAllocation = nullptr;
					std::vector<vk::WriteDescriptorSet>                      _updateImageWriteDescriptorSets;

					void SetWriteDescriptorSetDirty(size_t index);

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
			Vulkan::Pipeline _pipeline;
			std::string      _shaderPath;

			static Properties _globalProperties;
			static bool       _globalPropertiesDefined;

			Properties _shaderProperties;
	};

}