#include "SplitEngine/Utility/String.hpp"

namespace SplitEngine::Utility
{
	std::vector<std::string> String::Split(const std::string& str, char delimiter, int baseOffset)
	{
		std::vector<std::string> split;
		int                      offset         = baseOffset;
		int                      previousOffset = baseOffset;

		while (offset < str.length())
		{
			offset = str.find(delimiter, offset);

			// Break out of the loop if next find can't be found
			if (offset == std::string::npos) { offset = str.length(); }

			std::string sub = str.substr(previousOffset, (offset - previousOffset));
			split.push_back(sub);
			offset++;
			previousOffset = offset;
		}

		return split;
	}
}
