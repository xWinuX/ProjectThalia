#pragma once

#include <string>

namespace SplitEngine
{

	struct ApplicationInfo
	{
		public:
			std::string Name         = "Split Engine Game";
			uint32_t    MajorVersion = 1;
			uint32_t    MinorVersion = 0;
			uint32_t    PatchVersion = 0;

			// Shader stuff
			std::string VertexShaderFileExtension   = "vert";
			std::string FragmentShaderFileExtension = "frag";
			std::string SpirvFileExtension          = "spv";
	};

}
