#pragma once

#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

namespace ProjectThalia::Rendering
{
	class Renderer
	{
		public:
			Renderer() = default;

			void Initialize(Window& window);
			void DrawFrame();
			void Destroy();

		private:
			Vulkan::Context _vulkanContext;
	};
}
