#pragma once

#include "SplitEngine/Rendering/Vulkan/BufferFactory.hpp"
#include "SplitEngine/Rendering/Vulkan/Context.hpp"

namespace SplitEngine::Rendering
{
	class Model
	{
		public:
			struct CreateInfo
			{
				public:
					std::vector<std::byte> Vertices;
					std::vector<uint16_t>  Indices;
			};

		public:
			Model() = default;

			~Model();

			explicit Model(const CreateInfo& createInfo)
			{
				_modelBuffer = Vulkan::BufferFactory::CreateStagedModelBuffer(Vulkan::Context::GetDevice(), createInfo.Vertices, createInfo.Indices);
			}

			void Bind(vk::CommandBuffer& commandBuffer);

			[[nodiscard]] const Vulkan::Buffer& GetModelBuffer() const;

		private:
			Vulkan::Buffer _modelBuffer;
	};
}
