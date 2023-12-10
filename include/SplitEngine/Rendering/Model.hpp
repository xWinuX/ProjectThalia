#pragma once

#include "SplitEngine/Rendering/Vulkan/Buffer.hpp"
#include "SplitEngine/Rendering/Vulkan/Context.hpp"

namespace SplitEngine::Rendering
{
	class Model
	{
		public:
			Model() = default;

			~Model();

			template<typename TVertex>
			Model(std::vector<TVertex> vertices, std::vector<uint16_t> indices)
			{
				_modelBuffer = Vulkan::Buffer::CreateStagedModelBuffer(Vulkan::Context::GetDevice(), vertices, indices);
			}

			[[nodiscard]] const Vulkan::Buffer& GetModelBuffer() const;

		private:
			Vulkan::Buffer _modelBuffer;
	};
}
