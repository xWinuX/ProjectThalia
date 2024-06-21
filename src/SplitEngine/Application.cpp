#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "SplitEngine/Application.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Input.hpp"
#include "SplitEngine/Debug/Log.hpp"

#include <format>

#include <chrono>
#include <utility>

#include "SplitEngine/Contexts.hpp"
#include "SplitEngine/Stages.hpp"
#include "SplitEngine/Systems.hpp"

namespace SplitEngine
{
	Application::Application(CreateInfo createInfo):
		_applicationInfo(std::move(createInfo.ApplicationInfo)),
		_ecsSettings(std::move(createInfo.ECSSettings)),
		_renderer(Rendering::Renderer(_applicationInfo, std::move(createInfo.ShaderParserSettings), std::move(createInfo.RenderingSettings))),
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
		_ecsRegistry.AddSystem<StatisticsSystem>(EngineStage::BeginFrame, EngineStageOrder::BeginFrame_StatisticsSystem);
		_ecsRegistry.AddSystem<TimeSystem>(EngineStage::BeginFrame, EngineStageOrder::BeginFrame_TimeSystem);
		ECS::Registry::SystemHandle<SDLEventSystem> eventSystem = _ecsRegistry.AddSystem<SDLEventSystem>(EngineStage::BeginFrame, EngineStageOrder::BeginFrame_SDLEventSystem);
		_ecsRegistry.AddSystem<RenderingSystem>({
			                                        { EngineStage::BeginRendering, EngineStageOrder::BeginRendering_RenderingSystem },
			                                        { EngineStage::EndRendering, EngineStageOrder::EndRendering_RenderingSystem }
		                                        });

		_ecsRegistry.GetContextProvider().GetContext<EngineContext>()->EventSystem = eventSystem.System;
	}

	void Application::Run()
	{
		// Main Loop
		LOG("Start Game Loop");
		while (!_quit)
		{
			_ecsRegistry.ExecuteSystems(_ecsSettings.RootExecutionExecutePendingOperations, _ecsSettings.RootExecutionListBehaviour, _ecsSettings.RootExecutionStages);
		}

		LOG("Waiting for frame to finish...");
		_renderer.GetVulkanInstance().GetPhysicalDevice().GetDevice().WaitForIdle();
	}

	void Application::Quit() { _quit = true; }

	Window& Application::GetWindow() { return _renderer.GetWindow(); }

	AssetDatabase& Application::GetAssetDatabase() { return _assetDatabase; }

	ECS::Registry& Application::GetECSRegistry() { return _ecsRegistry; }
}
