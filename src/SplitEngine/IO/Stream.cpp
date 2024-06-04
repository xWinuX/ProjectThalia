#include "SplitEngine/IO/Stream.hpp"
#include "SplitEngine/ErrorHandler.hpp"

#include <filesystem>

namespace SplitEngine::IO
{
	Stream::Stream(const std::filesystem::path& filePath, StreamReadFormat readFormat) :
		_stream(std::ifstream(filePath, readFormat)),
		_readFormat(readFormat) { if (!_stream.is_open()) { ErrorHandler::ThrowRuntimeError(std::format("failed to open file {0}!", filePath.string())); } }

	void Stream::Close() { _stream.close(); }
}
