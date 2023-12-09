#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "ProjectThalia/Application.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Input.hpp"
#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <format>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ProjectThalia
{
	void Application::Initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
		{
			ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError()));
		}

		_window.Open();
		_renderer.Initialize(&_window);

		UserInitialize();
	}

	Application::~Application()
	{
		Rendering::Vulkan::Context::WaitForIdle();

		_window.Close();
	}

	void Application::Run()
	{
		Initialize();

		// Main Loop
		uint64_t currentTime = SDL_GetPerformanceCounter();
		uint64_t previousTime;
		float    deltaTime;

		uint64_t averageFps           = 0;
		float    averageDeltaTime     = 0.0f;
		float    accumulatedDeltaTime = 0;
		uint64_t accumulatedFrames    = 0;

		SDL_Event e;
		bool      quit = false;
		while (!quit)
		{
			previousTime = currentTime;
			currentTime  = SDL_GetPerformanceCounter();

			deltaTime = (static_cast<float>((currentTime - previousTime)) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;

			accumulatedDeltaTime += deltaTime;
			accumulatedFrames++;

			if (accumulatedDeltaTime >= 0.1f)
			{
				averageDeltaTime = accumulatedDeltaTime / static_cast<float>(accumulatedFrames);
				averageFps       = static_cast<uint64_t>((1.0f / averageDeltaTime));

				accumulatedDeltaTime = 0.0f;
				accumulatedFrames    = 0;
			}

			Input::Reset();

			while (SDL_PollEvent(&e))
			{
				ImGui_ImplSDL2_ProcessEvent(&e);

				switch (e.type)
				{
					case SDL_KEYDOWN:
					case SDL_KEYUP: Input::Update(e); break;
					case SDL_QUIT: quit = true; break;
				}

				switch (e.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED: _window.OnResize.Invoke(e.window.data1, e.window.data2); break;
					case SDL_WINDOWEVENT_MINIMIZED: _window.SetMinimized(true); break;
					case SDL_WINDOWEVENT_RESTORED: _window.SetMinimized(false); break;
				}
			}

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplSDL2_NewFrame(_window.GetSDLWindow());

			ImGui::NewFrame();

			ImGui::Text("DT: %f", averageDeltaTime);
			ImGui::Text("Frame Time (ms): %f", averageDeltaTime / 0.001f);
			ImGui::Text("FPS: %llu", averageFps);
			ImGui::Text("Num Quads: %llu", _numQuads);

			ImGui::Render();


			CameraUBO* cameraUbo = _material->GetDescriptorSetAllocation().ShaderBuffers[0].GetMappedData<CameraUBO>();

			cameraUbo->model = glm::rotate(glm::mat4(1.0f), (currentTime / 10000000.0f) * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			cameraUbo->view  = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			cameraUbo->proj  = glm::perspective(glm::radians(45.0f),
                                               static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().width) /
                                                       -static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().height),
                                               0.1f,
                                               10.0f);

			if (Input::GetPressed(SDL_KeyCode::SDLK_g)) { _material->SetTexture(0, *_floppaTexture); }
			if (Input::GetPressed(SDL_KeyCode::SDLK_h)) { _material->SetTexture(0, *_evilFloppaTexture); }

			//_material->GetDescriptorSetAllocation().ShaderBuffers[1].Invalidate();
			
			ObjectBuffer* objectBuffer = _material->GetDescriptorSetAllocation().ShaderBuffers[1].GetMappedData<ObjectBuffer>();

			for (ObjectData& objectData : objectBuffer->objects) { objectData.position += glm::vec4(0.0001f, 0.0001f, 0.0001f, 0.0f) ; }


			//_material->GetDescriptorSetAllocation().ShaderBuffers[1].Flush();

			if (!_window.IsMinimized())
			{
				_renderer.SubmitModel(_material.get(), _model.get());
				_renderer.Render();
			}
		}
	}

	void Application::UserInitialize()
	{
		_shader = std::make_unique<Rendering::Shader>("res/shaders/debug");

		Rendering::TextureSettings textureSettings {};
		_floppaTexture     = std::make_unique<Rendering::Texture2D>("res/textures/floppa.png", textureSettings);
		_evilFloppaTexture = std::make_unique<Rendering::Texture2D>("res/textures/evil_floppa.png", textureSettings);

		_material                  = std::make_unique<Rendering::Material>(_shader.get());
		ObjectBuffer* objectBuffer = _material->GetDescriptorSetAllocation().ShaderBuffers[1].GetMappedData<ObjectBuffer>();

		_numQuads = objectBuffer->objects.max_size();


		for (ObjectData& objectData : objectBuffer->objects) { objectData.position = glm::vec4(glm::ballRand(0.7f), 0.0f); }

		//_material->GetDescriptorSetAllocation().ShaderBuffers[1].Invalidate();
		//_material->GetDescriptorSetAllocation().ShaderBuffers[1].Flush();


		//_material->SetTexture(0, *_floppaTexture);
		//for (ObjectData& objectData : objectBuffer->objects) { objectData.model = glm::translate(glm::mat4(1.0), glm::ballRand(0.7f)); }
		//_material2 = std::make_unique<Rendering::Material>(_shader.get());

		const std::vector<Rendering::VertexPosition2DColorUV> quad        = {{{-0.0005f, 0.0005f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // Top Left
																			 {{0.0005f, 0.0005f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // Top Right
																			 {{-0.0005f, -0.0005f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom Left
																			 {{0.0005f, -0.0005f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}}; // Bottom Right
		const std::vector<uint16_t>                           quadIndices = {0, 1, 2, 2, 1, 3};

		std::vector<Rendering::VertexPosition2DColorUV> vertices;
		vertices.reserve(1024 * 4);
		std::vector<uint16_t> indices;
		indices.reserve(1024 * 6);
		for (size_t i = 0; i < 1024; i++)
		{
			for (const Rendering::VertexPosition2DColorUV& vertex : quad) { vertices.push_back(vertex); }

			for (uint16_t index : quadIndices) { indices.push_back(index + (i * 6)); }
		}

		_model = std::make_unique<Rendering::Model>(vertices, indices);
	}
}
