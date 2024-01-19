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
		 * in the pre rendering stage all gameplay systems have been executed fully
		 * It does not have a specific context
		 * Use this stage for heavy calculation that are needed by rendering systems
		 * This stage can't be used to submit data to the gpu since the frame may not have finished yet
		 */
		PreRendering = 1,
		/**
		 * In the rendering stage all gameplay systems and pre rendering systems have been executed fully
		 * The renderer has been prepared and awaited all pending actions so the commandBuffer inside the context object can be used to issue commands
		 * Use this stage to submit commands to the gpu
		 * This stage shouldn't be used to do heavy visual based calculations, do this in PreRendering Instead
		 */
		Rendering = 2,
		MAX_VALUE
	};
}