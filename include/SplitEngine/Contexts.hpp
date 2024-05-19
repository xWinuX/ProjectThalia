#pragma once

#include <vector>

namespace SplitEngine
{
	struct Statistics
	{
		uint64_t           AverageFPS            = 0;
		float              AverageDeltaTime      = 0.0f;
		std::vector<float> AverageECSStageTimeMs = std::vector<float>(std::numeric_limits<uint8_t>::max(), 0);
	};

	struct TimeContext
	{
		float DeltaTime = 0;
	};

	class Application;
	class AssetDatabase;

	struct EngineContext
	{
		Application*   Application   = nullptr;
		AssetDatabase* AssetDatabase = nullptr;
		Statistics     Statistics{};
		uint64_t       TestNumber = 42069;
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
