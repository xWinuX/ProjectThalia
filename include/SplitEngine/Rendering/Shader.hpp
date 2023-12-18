#pragma once

#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"

#include <string>

namespace SplitEngine::Rendering
{

	class Shader
	{
		public:
			struct CreateInfo
			{
				public:
					std::string ShaderPath;
			};
		public:
			Shader() = default;
			explicit Shader(const CreateInfo& createInfo);

			~Shader();

			[[nodiscard]] Vulkan::Pipeline& GetPipeline();

		private:
			Vulkan::Pipeline _pipeline;
			std::string      _shaderPath;
	};

}