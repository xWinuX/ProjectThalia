#pragma once

#include "DataStructures.hpp"
#include "Rendering/Vulkan/PipelineCreateInfo.hpp"
#include "Rendering/Vulkan/ViewportStyle.hpp"

namespace SplitEngine
{
	struct RenderingSettings
	{
		bool                                  UseVulkanValidationLayers = false;
		uint32_t                              GPUDeviceID               = -1u;
		Rendering::Vulkan::ViewportStyle      ViewportStyle             = Rendering::Vulkan::ViewportStyle::Flipped;
		Color                                 ClearColor                = Color(0x264E80FF);
		Rendering::Vulkan::PipelineCreateInfo PipelineCreateInfo{};
	};
}
