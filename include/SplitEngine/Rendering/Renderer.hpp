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
			void Render();

			void SubmitModel(Material* material, const Model* model);

			void HandleEvents(SDL_Event event);

		private:
			Window          _window;
			Vulkan::Context _vulkanContext;

			std::unordered_map<Material*, IncrementVector<const Model*>> _modelsToRender;

			bool _frameBufferResized = false;
			void StartImGuiFrame() const;
	};
}
