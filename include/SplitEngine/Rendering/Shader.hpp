#pragma once

#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"

#include <string>

namespace SplitEngine::Rendering
{

	class Shader
	{
		public:
			Shader() = default;
			explicit Shader(std::string shaderPath);

			~Shader();

			[[nodiscard]] Vulkan::Pipeline& GetPipeline();

		private:
			Vulkan::Pipeline _pipeline;
			std::string      _shaderPath;
	};

}