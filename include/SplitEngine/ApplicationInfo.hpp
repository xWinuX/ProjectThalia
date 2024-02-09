#pragma once

#include <string>

namespace SplitEngine
{
	struct ApplicationInfo
	{
		std::string Name         = "Split Engine Game";
		uint32_t    MajorVersion = 1;
		uint32_t    MinorVersion = 0;
		uint32_t    PatchVersion = 0;
	};
}
