#include "SplitEngine/IO/AudioLoader.hpp"
#include "SplitEngine/ErrorHandler.hpp"
#include "SplitEngine/Debug/Log.hpp"

namespace SplitEngine::IO
{
	Audio AudioLoader::LoadBuffer(const std::string& path) { return Load(path, AudioMemoryType::Buffer); }

	Audio AudioLoader::LoadStream(const std::string& path) { return Load(path, AudioMemoryType::Stream); }

	Audio AudioLoader::Load(const std::string& path, AudioLoader::AudioMemoryType memoryType)
	{
		Audio audio;

		unsigned int returnCode = 0;
		switch (memoryType)
		{

			case AudioMemoryType::Buffer:
				audio.Sample = new SoLoud::Wav();
				returnCode   = audio.Sample->load(path.c_str());
				break;
			case AudioMemoryType::Stream:
				audio.Stream = new SoLoud::WavStream();
				audio.Stream->load(path.c_str());
				break;
		}

		if (returnCode != 0) { ErrorHandler::ThrowRuntimeError(std::format("Failed to load sound: {0}", path)); }

		return audio;
	}
}
