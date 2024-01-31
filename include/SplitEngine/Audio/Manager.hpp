#pragma once

#define SOLOUD_NO_ASSERTS
#include <soloud.h>

#include "SoundEffect.hpp"
#include "SplitEngine/AssetDatabase.hpp"

namespace SplitEngine::Audio
{
	class Manager
	{

		public:
			Manager() = default;
			~Manager();
			void Initialize();

			void PlaySound(SoundEffect& soundEffect, float volume = 1.0f);
			void PlaySound(AssetHandle<SoundEffect>& soundEffect, float volume = 1.0f);

		private:
			SoLoud::Soloud* _audioEngine = nullptr;
	};

}
