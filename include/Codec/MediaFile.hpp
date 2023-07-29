#pragma once



#include <string>
#include "Strawberry/Core/Option.hpp"
#include "Packet.hpp"
#include "Decoder.hpp"
#include "Resampler.hpp"
#include "MediaStream.hpp"
#include <map>
#include <memory>
#include <Strawberry/Core/IO/Error.hpp>


extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}



namespace Strawberry::Codec
{
	class MediaFile
	{
		friend class MediaStream;


	public:
		static Core::Option<MediaFile> Open(const std::string& path);


		MediaFile(const MediaFile& other)            = delete;
		MediaFile& operator=(const MediaFile& other) = delete;
		MediaFile(MediaFile&& other) noexcept ;
		MediaFile& operator=(MediaFile&& rhs) noexcept ;
		~MediaFile();


		Core::Option<MediaStreamInfo> GetStreamInfo(size_t index);
		Core::Option<MediaStream*> GetStream(size_t index);


		Core::Option<MediaStream*> GetBestStream(MediaType type);


	protected:
		void Seek(size_t stream, size_t pts);
		Core::Result<Packet, Core::IO::Error> Read();


	private:
		MediaFile() = default;


	private:
		AVFormatContext* mFile = nullptr;
		std::map<size_t, MediaStream> mOpenStreams;
	};
}