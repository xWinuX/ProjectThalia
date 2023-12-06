#include "ProjectThalia/Rendering/Shader.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Context.hpp"
#include "ProjectThalia/Rendering/Vulkan/Pipeline.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

#include <utility>

namespace ProjectThalia::Rendering
{
	Shader::Shader(std::string shaderPath) :
		_shaderPath(shaderPath)
	{
		std::vector<std::filesystem::path> files;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(_shaderPath))
		{
			if (std::filesystem::is_regular_file(entry.path()) && entry.path().extension() == ".spv") { files.push_back(entry.path()); }
		}

		std::vector<Vulkan::Pipeline::ShaderInfo> shaderInfos;
		shaderInfos.reserve(files.size());

		// Get shader infos
		for (const auto& file : files)
		{
			LOG(file.string());
			size_t lastDotPosition       = file.string().rfind('.');
			size_t secondLastDotPosition = -1;

			// Find seconds last point
			for (size_t i = lastDotPosition - 1; i > 0; i--)
			{
				if (file.string()[i] == '.')
				{
					secondLastDotPosition = i;
					break;
				}
			}

			if (lastDotPosition == std::string::npos || secondLastDotPosition == std::string::npos)
			{
				ErrorHandler::ThrowRuntimeError(std::format("Failed to parse shader files! {0}", _shaderPath));
			}

			std::string                  shaderTypeString = file.string().substr(secondLastDotPosition + 1, lastDotPosition - secondLastDotPosition - 1);
			Vulkan::Pipeline::ShaderType shaderType       = Vulkan::Pipeline::ShaderType::Vertex;

			LOG(shaderTypeString);

			if (shaderTypeString == "vert") { shaderType = Vulkan::Pipeline::ShaderType::Vertex; }
			if (shaderTypeString == "frag") { shaderType = Vulkan::Pipeline::ShaderType::Fragment; }

			shaderInfos.push_back({file.string(), shaderType});
		}

		_pipeline = Vulkan::Pipeline(Vulkan::Context::GetDevice(), "main", shaderInfos);
	}

	Vulkan::Pipeline& Shader::GetPipeline() { return _pipeline; }

	Shader::~Shader() { _pipeline.Destroy(); }
}
