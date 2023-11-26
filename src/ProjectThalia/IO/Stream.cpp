#include "ProjectThalia/IO/Stream.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include <filesystem>

namespace ProjectThalia::IO
{
	Stream::Stream(const std::string& filePath, StreamReadFormat readFormat) :
		_stream(std::ifstream(filePath, readFormat)),
		_readFormat(readFormat)
	{
		if (!_stream.is_open()) { ErrorHandler::ThrowRuntimeError("failed to open file!"); }
	}

	void Stream::Close() { _stream.close(); }

}
