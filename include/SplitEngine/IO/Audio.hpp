#pragma once

#include "soloud_wav.h"
#include "soloud_wavstream.h"

namespace SplitEngine::IO
{
	struct Audio
	{
		public:
			~Audio()
			{
				delete Sample;
				delete Stream;
			}

			SoLoud::Wav*       Sample = nullptr;
			SoLoud::WavStream* Stream = nullptr;
	};
}
