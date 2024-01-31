#include "SplitEngine/Audio/Manager.hpp"

namespace SplitEngine::Audio
{
	Manager::~Manager()
	{
		LOG("Shutting down Audio...");
		_audioEngine->deinit();

		delete _audioEngine;
	}

	void Manager::Initialize()
	{
		_audioEngine = new SoLoud::Soloud();
		_audioEngine->init();
	}

	void Manager::PlaySound(SoundEffect& soundEffect, float volume) { _audioEngine->play(*soundEffect.GetAudio(), soundEffect.GetBaseVolume() * volume); }

	void Manager::PlaySound(AssetHandle<SoundEffect>& soundEffect, float volume) { PlaySound(*soundEffect.Get(), volume); }
}
