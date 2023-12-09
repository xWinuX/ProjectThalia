#pragma once
#define SDL_MAIN_HANDLED

#include "Event.hpp"
#include "ProjectThalia/Rendering/Material.hpp"
#include "ProjectThalia/Rendering/Model.hpp"
#include "ProjectThalia/Rendering/Renderer.hpp"
#include "Window.hpp"

#include <array>

namespace ProjectThalia
{
	class Application
	{
		public:
			Application() = default;
			~Application();
			void Run();


		private:
			struct CameraUBO
			{
					glm::mat4 model;
					glm::mat4 view;
					glm::mat4 proj;
			};

			struct ObjectData
			{
					glm::vec4 position;
			};

			struct ObjectBuffer
			{
					std::array<ObjectData, 1'024'000> objects;
			};

			void Initialize();
			void UserInitialize();

			size_t _numQuads;

			Event<int>          _event;
			Window              _window;
			Rendering::Renderer _renderer;

			std::unique_ptr<Rendering::Shader>    _shader;
			std::unique_ptr<Rendering::Material>  _material;
			std::unique_ptr<Rendering::Texture2D> _floppaTexture;
			std::unique_ptr<Rendering::Texture2D> _evilFloppaTexture;

			std::unique_ptr<Rendering::Model> _model;
	};
}