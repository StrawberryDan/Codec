#pragma once


// This Project
#include "Codec/Audio/Decoder.hpp"
#include "Codec/Audio/Resampler.hpp"
#include "Codec/MediaStream.hpp"
#include "Codec/Packet.hpp"
// Strawberry Core
#include "Strawberry/Core/Types/Optional.hpp"
#include "Strawberry/Core/IO/Error.hpp"
#include "Strawberry/Core/Types/ReflexivePointer.hpp"

// FFMPEG
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

// Standard Library
#include <map>
#include <filesystem>
#include <memory>
#include <string>

namespace Strawberry::Codec
{
	class MediaFile
			: public Core::EnableReflexivePointer
	{
		friend class MediaStream;

	public:
		static Core::Optional<MediaFile> Open(const std::filesystem::path& path);


		MediaFile(const MediaFile& other)            = delete;
		MediaFile& operator=(const MediaFile& other) = delete;
		MediaFile(MediaFile&& other) noexcept;
		MediaFile& operator=(MediaFile&& rhs) noexcept;
		~MediaFile();


		const std::filesystem::path& GetPath() const;


		Core::Optional<MediaStreamInfo>     GetStreamInfo(size_t index);
		Core::ReflexivePointer<MediaStream> GetStream(size_t index);


		Core::ReflexivePointer<MediaStream> GetBestStream(MediaType type);

	protected:
		Core::Result<Packet, Core::IO::Error> Read();

	private:
		MediaFile() = default;

	private:
		std::filesystem::path         mPath;
		AVFormatContext*              mFile = nullptr;
		std::map<size_t, MediaStream> mOpenStreams;
	};
} // namespace Strawberry::Codec
