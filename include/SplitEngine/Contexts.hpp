#pragma once

#include <vector>

namespace SplitEngine
{
	struct Statistics
	{
		uint64_t           AverageFPS            = 0;
		float              AverageDeltaTime      = 0.0f;
		std::vector<float> AverageECSStageTimeMs = std::vector<float>(UINT8_MAX, 0); // numeric_limits not used because soloud overrides the max function by importing some fuckass windows library
	};

	struct TimeContext
	{
		float DeltaTime = 0;
	};

	class Application;
	class AssetDatabase;
	class SDLEventSystem;

	struct EngineContext
	{
		Application*    Application   = nullptr;
		AssetDatabase*  AssetDatabase = nullptr;
		Statistics      Statistics{};
		SDLEventSystem* EventSystem = nullptr;
	};

	namespace Rendering
	{
		class Renderer;
	}

	struct RenderingContext
	{
		Rendering::Renderer* Renderer = nullptr;
	};

	namespace Audio
	{
		class Manager;
	}

	struct AudioContext
	{
		Audio::Manager* AudioManager = nullptr;
	};
}
