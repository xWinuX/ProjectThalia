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

			static std::vector<char> ReadRawAndClose(const std::string& filePath, StreamReadFormat format = StreamReadFormat::Default);

			std::vector<char> ReadRaw();

			void Close();

		private:
			std::ifstream _stream;
			StreamReadFormat _readFormat = StreamReadFormat::Default;
	};
}