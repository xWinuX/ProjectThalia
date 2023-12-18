#include "SplitEngine/Rendering/Shader.hpp"
#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Rendering/Vulkan/Context.hpp"
#include "SplitEngine/Rendering/Vulkan/Pipeline.hpp"

#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

namespace SplitEngine::Rendering
{
	Shader::Shader(const CreateInfo& createInfo) :
		_shaderPath(std::move(createInfo.ShaderPath))
	{
		std::vector<std::filesystem::path> files;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(_shaderPath))
		{
			bool hasSpvExtension = entry.path().extension() == std::format(".{0}", Application::GetApplicationInfo().SpirvFileExtension);
			if (std::filesystem::is_regular_file(entry.path()) && hasSpvExtension) { files.push_back(entry.path()); }
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

			if (shaderTypeString == Application::GetApplicationInfo().VertexShaderFileExtension) { shaderType = Vulkan::Pipeline::ShaderType::Vertex; }
			if (shaderTypeString == Application::GetApplicationInfo().FragmentShaderFileExtension) { shaderType = Vulkan::Pipeline::ShaderType::Fragment; }

			shaderInfos.push_back({file.string(), shaderType});
		}

		_pipeline = Vulkan::Pipeline(Vulkan::Context::GetDevice(), "main", shaderInfos);
	}

	Vulkan::Pipeline& Shader::GetPipeline() { return _pipeline; }

	Shader::~Shader() { _pipeline.Destroy(); }
}
