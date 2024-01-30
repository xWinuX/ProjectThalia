#include "SplitEngine/IO/Stream.hpp"
#include "SplitEngine/ErrorHandler.hpp"

#include <filesystem>

namespace SplitEngine::IO
{
	Stream::Stream(const std::string& filePath, const StreamReadFormat readFormat) :
		_stream(std::ifstream(filePath, readFormat)),
		_readFormat(readFormat) { if (!_stream.is_open()) { ErrorHandler::ThrowRuntimeError("failed to open file!"); } }

	void Stream::Close() { _stream.close(); }
}
