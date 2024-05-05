#pragma once
#include <vulkan/vulkan_core.h>

namespace SplitEngine::Rendering
{
	enum class ShaderType
	{
		Vertex   = VK_SHADER_STAGE_VERTEX_BIT,
		Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
		Compute  = VK_SHADER_STAGE_COMPUTE_BIT
	};
}
