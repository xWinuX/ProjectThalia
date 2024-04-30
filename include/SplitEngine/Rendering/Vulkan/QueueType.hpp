#pragma once
#include <cstdint>

namespace SplitEngine::Rendering::Vulkan
{
	enum class QueueType : uint8_t
	{
		Graphics = 0,
		Present  = 1,
		Transfer = 2,
		Compute  = 3,
		MAX_VALUE
	};
}
