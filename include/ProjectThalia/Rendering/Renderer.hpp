#pragma once

#include "Material.hpp"
#include "Model.hpp"

#include "Vulkan/Context.hpp"

#include "ProjectThalia/DataStructures.hpp"

namespace ProjectThalia::Rendering
{
	class Renderer
	{
		public:
			Renderer() = default;
			~Renderer();

			void Initialize(Window* window);
			void Render();

			void SubmitModel(const Material* material, const Model* model);

		private:
			Window*         _window;
			Vulkan::Context _vulkanContext;

			std::unordered_map<const Material*, IncrementVector<const Model*>> _modelsToRender;

			bool _frameBufferResized = false;
	};
}
