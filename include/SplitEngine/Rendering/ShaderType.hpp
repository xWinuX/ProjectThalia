#pragma once
#include <vulkan/vulkan_core.h>

namespace SplitEngine::Rendering
{
	enum class ShaderType
	{
		Vertex   = 0,
		Fragment = 1,
		Compute  = 2,
		MAX_VALUE,
	};
}
