#pragma once

#include <vector>

namespace SplitEngine::IO
{
	struct Image
	{
		public:
			std::vector<std::byte> Pixels {};

			uint32_t Width    = 0;
			uint32_t Height   = 0;
			uint32_t Channels = 4;
	};
}