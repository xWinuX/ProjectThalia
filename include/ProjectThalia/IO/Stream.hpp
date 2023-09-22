#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>


namespace ProjectThalia::IO
{
	struct StreamReadFormat {
			int Format;
	};
	class Stream
	{
		public:
			explicit Stream(std::string filePath, StreamReadFormat readFormat);

			class ReadFormat {
					static StreamReadFormat Binary;
			};

			std::vector<char> ReadRaw();

			static std::vector<char> ReadRawAndClose(const std::string& filePath, StreamReadFormat format);

			void Close();

		private:
			std::ifstream _stream;
			StreamReadFormat _readFormat;
	};

	inline StreamReadFormat Stream::ReadFormat::Binary = {std::ios::ate | std::ios::binary};
}