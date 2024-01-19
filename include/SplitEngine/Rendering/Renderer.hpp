#pragma once

#include "Material.hpp"
#include "Model.hpp"

#include "Vulkan/Context.hpp"

#include "SplitEngine/DataStructures.hpp"

namespace SplitEngine::Rendering
{
	class Renderer
	{
		public:
			Renderer() = default;
			~Renderer();

			void Initialize();

			void BeginRender();
			void EndRender();

			void HandleEvents(SDL_Event event) const;

			Vulkan::Context& GetContext();

		private:
			Window          _window {};
			Vulkan::Context _vulkanContext {};


			bool     _wasSkipped             = false;
			uint32_t _latestImageIndexResult = 0;

			bool _frameBufferResized = false;
			void StartImGuiFrame() const;
	};
}
