#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace SplitEngine::IO
{
	enum StreamReadFormat
	{
		Default = 0,
		Binary  = std::ios::ate | std::ios::binary
	};

	class Stream
	{
		public:
			explicit Stream(const std::filesystem::path& filePath, StreamReadFormat readFormat = StreamReadFormat::Default);

			template<typename T = char>
			static std::vector<T> ReadRawAndClose(const std::filesystem::path& filePath, StreamReadFormat format = StreamReadFormat::Default)
			{
				Stream         stream = Stream(filePath, format);
				std::vector<T> buffer = stream.ReadRaw<T>();
				stream.Close();

				return buffer;
			}

			template<typename T = char>
			std::vector<T> ReadRaw()
			{
				const size_t   fileSize = _stream.tellg();
				std::vector<T> buffer(fileSize / sizeof(T));

				_stream.seekg(0);
				_stream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));

				_stream.close();

				return buffer;
			}

			void Close();

		private:
			std::ifstream    _stream;
			StreamReadFormat _readFormat = StreamReadFormat::Default;
	};
}
