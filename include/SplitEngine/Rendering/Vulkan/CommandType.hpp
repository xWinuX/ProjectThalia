#pragma once

namespace SplitEngine::Rendering::Vulkan
{
	enum CommandType
	{
		Graphics = 0,
		Present  = 1,
		Transfer = 2,
		Compute  = 3,
		MAX_VALUE
	};
}
