#pragma once

namespace SplitEngine::ECS
{
	enum class Stage : uint8_t
	{
		/**
		 * The gameplay stage is the first stage to execute on the beginning of a new frame
		 * It does not have a specific context
		 */
		Gameplay = 0,

		/**
		 * In the rendering stage all gameplay systems and pre rendering systems have been executed fully
		 * The renderer has been prepared and awaited all pending actions so the commandBuffer inside the context object can be used to issue commands
		 * Use this stage to submit commands to the gpu
		 */
		Rendering = 1,

		/**
		 * DO NOT USE
		 */
		MAX_VALUE
	};
}