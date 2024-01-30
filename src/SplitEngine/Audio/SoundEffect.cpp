#include "SplitEngine/Audio/SoundEffect.hpp"

namespace SplitEngine::Audio
{
	SoundEffect::SoundEffect(SoundEffect::CreateInfo&& createInfo)
	{
		_audio      = createInfo.Audio;
		_baseVolume = createInfo.BaseVolume;

		createInfo.Audio.Sample = nullptr;
		createInfo.Audio.Stream = nullptr;
	}

	float SoundEffect::GetBaseVolume() const { return _baseVolume; }

	SoLoud::AudioSource* SoundEffect::GetAudio() const
	{
		if (_audio.Sample != nullptr) { return _audio.Sample; }
		return _audio.Stream;
	}
}
