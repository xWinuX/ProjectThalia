#pragma once
#define SDL_MAIN_HANDLED

#include "Event.hpp"
#include "ProjectThalia/Rendering/Material.hpp"
#include "ProjectThalia/Rendering/Model.hpp"
#include "ProjectThalia/Rendering/Renderer.hpp"
#include "Window.hpp"

namespace ProjectThalia
{
	class Application
	{
		public:
			Application() = default;
			void Run();


		private:
			struct CameraUBO
			{
					glm::mat4 model;
					glm::mat4 view;
					glm::mat4 proj;
			};

			void Initialize();
			void UserInitialize();
			void Destroy();

			Rendering::Shader*   _shader;
			Rendering::Material* _material;
			Rendering::Model*    _model;

			Event<int>          _event;
			Window              _window;
			Rendering::Renderer _renderer;
			Rendering::Material* _material2;
	};
}