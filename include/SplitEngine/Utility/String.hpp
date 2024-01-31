#pragma once

#include <string>
#include <vector>

namespace SplitEngine::Utility
{

	class String
	{
		public:
			static std::vector<std::string> Split(const std::string& str, char delimiter, int baseOffset);
	};

}
