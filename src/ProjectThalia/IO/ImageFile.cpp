#include "ProjectThalia/IO/ImageFile.hpp"

#include "ProjectThalia/ErrorHandler.hpp"

namespace ProjectThalia::IO
{
	ImageFile::ImageFile(std::string filePath, ChannelSetup channels) :
		_filePath(std::move(filePath))
	{
		_pixels = stbi_load(_filePath.c_str(), &_width, &_height, &_channels, channels);
		if (_pixels == nullptr) { ErrorHandler::ThrowRuntimeError("failed to load texture image!"); }

		int numChannels = 0;
		switch (_channels)
		{
			case ChannelSetup::RGB: numChannels = 3; break;
			case ChannelSetup::RGBA: numChannels = 4; break;
			case ChannelSetup::Mono: numChannels = 1; break;
			case ChannelSetup::MonoAlpha: numChannels = 2; break;
		}

		_totalImageSize = _width * _height * numChannels;
	}

	ImageFile::~ImageFile()
	{
		if (_pixels) { stbi_image_free(_pixels); }
	}

	const std::string& ImageFile::GetFilePath() const { return _filePath; }

	const stbi_uc* ImageFile::GetPixels() const { return _pixels; }

	int ImageFile::GetWidth() const { return _width; }

	int ImageFile::GetHeight() const { return _height; }

	int ImageFile::GetChannels() const { return _channels; }

	uint64_t ImageFile::GetTotalImageSize() const { return _totalImageSize; }
}
