#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Input.hpp"
#include "SplitEngine/Debug/Log.hpp"

#include <format>

#include <chrono>
#include <utility>

#include "SplitEngine/Contexts.hpp"
#include "SplitEngine/Stage.hpp"
#include "SplitEngine/Systems.hpp"


#include <imgui.h>


namespace SplitEngine
{
	Application::Application(CreateInfo createInfo):
		_applicationInfo(std::move(createInfo.ApplicationInfo)),
		_renderer(Rendering::Renderer(_applicationInfo, std::move(createInfo.RenderingSettings))),
		_audioManager(Audio::Manager())

	{
		LOG("Initializing SDL Events...");
		if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) { ErrorHandler::ThrowRuntimeError(std::format("SDL could not initialize! SDL_Error: {0}\n", SDL_GetError())); }

		LOG("Initializing ECS...");
		LOG("Registering Engine Contexts...");
		_ecsRegistry.RegisterContext<EngineContext>({ this, &_assetDatabase, {} });
		_ecsRegistry.RegisterContext<TimeContext>({});

		_ecsRegistry.RegisterContext<RenderingContext>({ &_renderer });
		_ecsRegistry.RegisterContext<AudioContext>({ &_audioManager });


		LOG("Adding Engine Systems...");
		_ecsRegistry.AddSystem<StatisticsSystem>(Stage::BeginFrame, Order::BeginFrame_StatisticsSystem);
		_ecsRegistry.AddSystem<TimeSystem>(Stage::BeginFrame, Order::BeginFrame_TimeSystem);
		_ecsRegistry.AddSystem<SDLEventSystem>(Stage::BeginFrame, Order::BeginFrame_SDLEventSystem);
		_ecsRegistry.AddSystem<RenderingSystem>({ { Stage::BeginRendering, Order::BeginRendering_RenderingSystem }, { Stage::EndRendering, Order::EndRendering_RenderingSystem } });
	}

	void Application::Run()
	{
		// Main Loop
		LOG("Start Game Loop");
		while (!_quit)
		{
			_ecsRegistry.ExecuteSystems();
		}

		LOG("Waiting for frame to finish...");
		_renderer.GetVulkanInstance().GetPhysicalDevice().GetDevice().WaitForIdle();
	}

	void Application::Quit() { _quit = true; }

	Window& Application::GetWindow() { return _renderer.GetWindow(); }

	AssetDatabase& Application::GetAssetDatabase() { return _assetDatabase; }

	ECS::Registry& Application::GetECSRegistry() { return _ecsRegistry; }
}
