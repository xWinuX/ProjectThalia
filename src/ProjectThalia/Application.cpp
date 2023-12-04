#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "ProjectThalia/Application.hpp"
#include "ProjectThalia/Debug/Log.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include "ProjectThalia/Rendering/Vulkan/Context.hpp"

#include <format>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <chrono>

namespace ProjectThalia
{
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

			while (SDL_PollEvent(&e))
			{
				ImGui_ImplSDL2_ProcessEvent(&e);

				switch (e.type)
				{
					case SDL_KEYDOWN: _event.Invoke(0); break;
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

			ImGui::Render();


			CameraUBO* cameraUbo = _material->GetDescriptorSetAllocation().ShaderBuffers[0].GetMappedData<CameraUBO>();

			cameraUbo->model = glm::rotate(glm::mat4(1.0f), (currentTime / 10000000.0f) * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			cameraUbo->view  = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			cameraUbo->proj  = glm::perspective(glm::radians(45.0f),
                                               static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().width) /
                                                       -static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().height),
                                               0.1f,
                                               10.0f);

			CameraUBO* cameraUbo2 = _material2->GetDescriptorSetAllocation().ShaderBuffers[0].GetMappedData<CameraUBO>();

			cameraUbo2->model = glm::translate(glm::rotate(glm::mat4(1.0f), (currentTime / 10000000.0f) * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
											   glm::vec3(1.0, 0.0, 0.0));
			cameraUbo2->view  = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			cameraUbo2->proj  = glm::perspective(glm::radians(45.0f),
                                                static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().width) /
                                                        -static_cast<float>(Rendering::Vulkan::Context::GetDevice()->GetSwapchain().GetExtend().height),
                                                0.1f,
                                                10.0f);


			if (!_window.IsMinimized())
			{
				_renderer.SubmitModel(_material, _model);
				_renderer.SubmitModel(_material2, _model);
				_renderer.Render();
			}
		}

		Destroy();
	}

	void Application::Destroy()
	{
		Rendering::Vulkan::Context::WaitForIdle();

		delete _material;
		delete _material2;
		delete _shader;
		delete _model;

		this->_renderer.Destroy();
		this->_window.Close();
	}

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

	void Application::UserInitialize()
	{
		LOG("User init begin");

		LOG("before shader");
		_shader = new Rendering::Shader("res/shaders/debug");

		LOG("before material");
		_material  = new Rendering::Material(_shader);
		_material2 = new Rendering::Material(_shader);

		LOG("vectoreooasdj");
		const std::vector<Rendering::VertexPosition2DColorUV> vertices = {{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // Top Left
																		  {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // Top Right
																		  {{-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom Left
																		  {{0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}}; // Bottom Right

		const std::vector<uint16_t> indices = {0, 1, 2, 2, 1, 3};

		LOG("before model");
		_model = new Rendering::Model(vertices, indices);
		LOG("User init end");
	}

}
