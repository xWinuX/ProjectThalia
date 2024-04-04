#pragma once

#include <string>
#include <vector>

namespace SplitEngine
{
	struct RenderingSettings
	{
		std::string VertexShaderFileExtension   = "vert";
		std::string FragmentShaderFileExtension = "frag";
		std::string ComputeShaderFileExtension  = "comp";
		std::string SpirvFileExtension          = "spv";
		char        ShaderBufferModDelimiter    = '_';

		std::vector<std::string> ShaderBufferSingleInstanceModPrefixes = {"singleInst", "si"};
		std::vector<std::string> ShaderBufferDeviceLocalModPrefixes    = {"deviceLocal", "dl"};
		std::vector<std::string> ShaderBufferCacheModPrefixes          = {"chache", "c"};

		bool UseVulkanValidationLayers = false;
	};
}
