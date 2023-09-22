#include "Stream.hpp"
#include "ProjectThalia/ErrorHandler.hpp"
#include <filesystem>

namespace ProjectThalia::IO
{

	std::vector<char> Stream::ReadRaw()
	{
		size_t            fileSize = (size_t) _stream.tellg();
		std::vector<char> buffer(fileSize);

		_stream.seekg(0);
		_stream.read(buffer.data(), static_cast<>fileSize);

		_stream.close();

		return buffer;
	}

	std::vector<char> Stream::ReadRawAndClose(const std::string& filePath, StreamReadFormat format)
	{
		Stream            stream = Stream(filePath, format);
		std::vector<char> buffer = stream.ReadRaw();
		stream.Close();

		return buffer;
	}

	void Stream::Close() {
		_stream.close();
	}

	Stream::Stream(std::string filePath, StreamReadFormat readFormat) :
		_stream(std::ifstream(filePath, readFormat.Format)),
		_readFormat(readFormat)
	{
		if (!_stream.is_open()) { ErrorHandler::ThrowRuntimeError("failed to open file!"); }
	}
}
