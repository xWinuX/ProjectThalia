#pragma once
#include <stb_image.h>
#include <string>

namespace ProjectThalia::IO
{
	class ImageFile
	{
		public:
			enum ChannelSetup
			{
				Mono      = STBI_grey,
				MonoAlpha = STBI_grey_alpha,
				RGB       = STBI_rgb,
				RGBA      = STBI_rgb_alpha
			};

		public:
			explicit ImageFile(std::string filePath, ChannelSetup channels = ChannelSetup::RGB);
			~ImageFile();

			void Destroy();

			[[nodiscard]] const std::string& GetFilePath() const;
			[[nodiscard]] const stbi_uc*     GetPixels() const;
			[[nodiscard]] int                GetWidth() const;
			[[nodiscard]] int                GetHeight() const;
			[[nodiscard]] int                GetChannels() const;
			[[nodiscard]] uint64_t           GetTotalImageSize() const;

		private:
			std::string _filePath;

			stbi_uc* _pixels         = nullptr;
			int      _width          = 0;
			int      _height         = 0;
			int      _channels       = 0;
			uint64_t _totalImageSize = 0;
	};

}
