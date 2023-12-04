#pragma once

#include "ProjectThalia/Rendering/Vulkan/Buffer.hpp"
#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

namespace ProjectThalia::Rendering
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
