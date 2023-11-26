#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace ProjectThalia::IO
{
	enum StreamReadFormat
	{
		Default = 0,
		Binary  = std::ios::ate | std::ios::binary
	};

	class Stream
	{
		public:
			explicit Stream(const std::string& filePath, StreamReadFormat readFormat = StreamReadFormat::Default);

			template<typename T = char>
			static std::vector<T> ReadRawAndClose(const std::string& filePath, StreamReadFormat format = StreamReadFormat::Default)
			{
				Stream         stream = Stream(filePath, format);
				std::vector<T> buffer = stream.ReadRaw<T>();
				stream.Close();

				return buffer;
			}

			template<typename T = char>
			std::vector<T> ReadRaw()
			{
				size_t         fileSize = (size_t) _stream.tellg();
				std::vector<T> buffer(fileSize / sizeof(T));

				_stream.seekg(0);
				_stream.read((char*) buffer.data(), static_cast<std::streamsize>(fileSize));

				_stream.close();

				return buffer;
			}

			void Close();

		private:
			std::ifstream    _stream;
			StreamReadFormat _readFormat = StreamReadFormat::Default;
	};
}