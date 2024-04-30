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

					~Properties();

					template<typename T>
					[[nodiscard]] Vulkan::InFlightResource<std::byte*>& GetBufferRaw(const uint32_t bindingPoint) const { return GetDescriptor(bindingPoint)->BufferPtrs; }

					template<typename T>
					[[nodiscard]] T* GetBufferData(const uint32_t bindingPoint) { return reinterpret_cast<T*>(GetDescriptor(bindingPoint)->BufferPtrs.Get()); }

					template<typename T>
					[[nodiscard]] T* GetBufferData(const uint32_t bindingPoint, uint32_t frameInFlight) { return reinterpret_cast<T*>(GetDescriptor(bindingPoint)->BufferPtrs[frameInFlight]); }

					template<typename T>
					[[nodiscard]] T** GetStorableBufferData(uint32_t bindingPoint) { return reinterpret_cast<T**>(GetDescriptor(bindingPoint)->BufferPtrs.GetDataPtr()); }

					[[nodiscard]] Vulkan::Buffer&                 GetBuffer(uint32_t bindingPoint) const;
					[[nodiscard]] const vk::DescriptorBufferInfo& GetBufferInfo(uint32_t bindingPoint, uint32_t frameInFlight = -1) const;

					void SetTexture(uint32_t bindingPoint, const Texture2D& texture);
					void SetTextures(uint32_t bindingPoint, size_t offset, std::vector<std::unique_ptr<Texture2D>>& textures);
					void SetTextures(uint32_t bindingPoint, size_t offset, std::vector<AssetHandle<Texture2D>>& textures);

					void SetBuffer(uint32_t bindingPoint, const Vulkan::Buffer& buffer, size_t offset, size_t range, uint32_t frameInFlight = -1);

					/**
					 * Override backing buffer of this binding point
					 * This also set the binding point dirty and changes the buffer infos to use assigned Buffer
					 * The Buffer is now owned by the backing descriptor and will be destroyed when the descriptor leaves the scope
					 */
					void OverrideBuffer(uint32_t bindingPoint, Vulkan::Buffer&& buffer);

					void OverrideBufferPtrs(uint32_t bindingPoint, Vulkan::Buffer& buffer);

				private:
					struct SharedPropertyEntry
					{
						struct PropertyEntry
						{
							uint32_t    BindingPoint = -1;
							Properties* Property     = nullptr;
						};

						std::vector<PropertyEntry> Propertieses{};
						AvailableStack<uint32_t>   AvailableStack{};
					};

					uint32_t                                          _id;
					Shader*                                           _shader                  = nullptr;
					Vulkan::DescriptorSetAllocator::Allocation*       _descriptorSetAllocation = nullptr;
					std::vector<Vulkan::InFlightResource<std::byte*>> _shaderBuffers{};
					std::vector<vk::WriteDescriptorSet>               _updateWriteDescriptorSets{};
					std::vector<uint32_t>                             _sharedPtrRemoveIndex;

					static uint32_t                         _idCounter;
					static std::vector<SharedPropertyEntry> _sharedProperties;

					[[nodiscard]] Vulkan::Descriptor* GetDescriptor(uint32_t bindingPoint) const;

					void SetWriteDescriptorSetDirty(uint32_t bindingPoint, uint32_t frameInFlight = -1);
					void SetSharedWriteDescriptorsDirty(Vulkan::Descriptor* descriptor, uint32_t frameInFlight = -1);

					void Update();
			};

		public:
			Shader() = default;
			explicit Shader(const CreateInfo& createInfo);

			~Shader();

			void BindGlobal(const vk::CommandBuffer& commandBuffer, uint32_t frameInFlight = -1) const;

			void Bind(const vk::CommandBuffer& commandBuffer, uint32_t frameInFlight = -1) const;

			static void UpdateGlobal();

			void Update();

			Properties&        GetProperties();
			static Properties& GetGlobalProperties();

			[[nodiscard]] Vulkan::Pipeline& GetPipeline();

		private:
			Vulkan::Device*  _device = nullptr;
			Vulkan::Pipeline _pipeline{};
			std::string      _shaderPath{};

			static Properties _globalProperties;
			static bool       _globalPropertiesDefined;

			Properties _shaderProperties{};
	};
}
