#pragma once


// This Project
#include "Codec/MediaStream.hpp"
#include "Codec/Packet.hpp"
// Strawberry Core
#include "Error.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "Strawberry/Core/IO/Error.hpp"
#include "Strawberry/Core/Types/ReflexivePointer.hpp"

// FFMPEG
extern "C" {
#include "libavformat/avformat.h"
}

// Standard Library
#include <map>
#include <filesystem>

namespace Strawberry::Codec
{
	class MediaFile
	{
		friend class MediaStream;

	public:
		static Core::Result<MediaFile, Error> Open(const std::filesystem::path& path);


		MediaFile(const MediaFile& other) = delete;
		MediaFile& operator=(const MediaFile& other) = delete;
		MediaFile(MediaFile&& other) noexcept;
		MediaFile& operator=(MediaFile&& rhs) noexcept;
		~MediaFile();


		const std::filesystem::path& GetPath() const;


		MediaStreamInfo GetStreamInfo(size_t index) const;
		MediaStream GetStream(size_t index) const;


		Core::Optional<MediaStream> GetBestStream(MediaType type) const;


	private:
		MediaFile() = default;

	private:
		std::filesystem::path mPath;
		AVFormatContext* mFile = nullptr;
		std::map<size_t, MediaStream> mOpenStreams;
	};
} // namespace Strawberry::Codec
