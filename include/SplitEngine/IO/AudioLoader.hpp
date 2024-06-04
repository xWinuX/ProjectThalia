#pragma once

#include <filesystem>
#include <string>
#include "SplitEngine/IO/Audio.hpp"

namespace SplitEngine::IO
{
	class AudioLoader
	{
		public:
			enum class AudioMemoryType
			{
				Buffer, // Load whole audio into memory
				Stream  // Stream audio
			};

			static Audio Load(const std::filesystem::path& path, AudioMemoryType memoryType);
			static Audio LoadBuffer(const std::filesystem::path& path);
			static Audio LoadStream(const std::filesystem::path& path);
	};
}
