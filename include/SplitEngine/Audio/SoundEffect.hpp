#pragma once

#include "SplitEngine/IO/Audio.hpp"

namespace SoLoud
{
	class AudioSource;
}

namespace SplitEngine::Audio
{
	class SoundEffect
	{
		public:
			struct CreateInfo
			{
				public:
					IO::Audio Audio {};
					float     BaseVolume = 0.5f;
			};

		public:
			explicit SoundEffect(CreateInfo&& createInfo);

			[[nodiscard]] SoLoud::AudioSource* GetAudio() const;
			[[nodiscard]] float                GetBaseVolume() const;

		private:
			IO::Audio _audio;
			float     _baseVolume;
	};
}
