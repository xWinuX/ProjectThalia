#pragma once
#include <cstdint>

namespace SplitEngine
{
	enum EngineStage : uint8_t
	{
		/**
		 * The Begin Frame Stage executes stuff like delta time calculation and input polling
		 */
		BeginFrame = 20,

		/**
		 * This stage is used to start rendering (renderpass getting binded etc...)
		 */
		BeginRendering = 150,

		/**
		 * This stage ends the rendering process (renderpass getting unbinded and swapchain image retrieval etc...)
		 */
		EndRendering = 200,
	};

	enum EngineStageOrder
	{
		BeginFrame_StatisticsSystem    = -11'000,
		BeginFrame_TimeSystem          = -10'000,
		BeginFrame_SDLEventSystem      = -10'000,
		BeginRendering_RenderingSystem = -10'000,
		EndRendering_RenderingSystem   = -10'000,
	};
}
