#pragma once


#include "Codec/Audio/Decoder.hpp"
#include "Codec/Audio/Resampler.hpp"
#include "MediaStream.hpp"
#include "Packet.hpp"
#include "Strawberry/Core/Util/Optional.hpp"
#include <Strawberry/Core/IO/Error.hpp>
#include <map>
#include <memory>
#include <string>


extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

namespace Strawberry::Codec
{
	class MediaFile
	{
		friend class MediaStream;


	public:
		static Core::Optional<MediaFile> Open(const std::string& path);


		MediaFile(const MediaFile& other)            = delete;
		MediaFile& operator=(const MediaFile& other) = delete;
		MediaFile(MediaFile&& other) noexcept;
		MediaFile& operator=(MediaFile&& rhs) noexcept;
		~MediaFile();


		Core::Optional<MediaStreamInfo> GetStreamInfo(size_t index);
		Core::Optional<MediaStream*>    GetStream(size_t index);


		Core::Optional<MediaStream*> GetBestStream(MediaType type);


	protected:
		void                                  Seek(size_t stream, size_t pts);
		Core::Result<Packet, Core::IO::Error> Read();


	private:
		MediaFile() = default;


	private:
		AVFormatContext*              mFile = nullptr;
		std::map<size_t, MediaStream> mOpenStreams;
	};
} // namespace Strawberry::Codec