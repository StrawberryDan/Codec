#include "Codec/MediaFile.hpp"
#include "Strawberry/Core/Util/Utilities.hpp"



#include <iostream>
#include <Strawberry/Core/Util/Markers.hpp>
#include <Strawberry/Core/IO/Error.hpp>


using Strawberry::Core::Assert;
using Strawberry::Core::Take;
using Strawberry::Core::Replace;



namespace Strawberry::Codec
{
	Core::Option<MediaFile> MediaFile::Open(const std::string& path)
	{
		MediaFile file;

		auto result = avformat_open_input(&file.mFile, path.c_str(), nullptr, nullptr);
		if (result != 0) return Core::NullOpt;

		result = avformat_find_stream_info(file.mFile, nullptr);
		if (result != 0) return Core::NullOpt;

		return file;
	}



	MediaFile::MediaFile(MediaFile&& other) noexcept
		: mFile(std::exchange(other.mFile, nullptr))
	{}



	MediaFile& MediaFile::operator=(MediaFile&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return (*this);
	}



	MediaFile::~MediaFile()
	{
		if (mFile)
		{
			avformat_close_input(&mFile);
		}
	}


	Core::Option<MediaStreamInfo> MediaFile::GetStreamInfo(size_t index)
	{
		if (index >= mFile->nb_streams) return Core::NullOpt;

		MediaStreamInfo info{};

		switch (mFile->streams[index]->codecpar->codec_type)
		{
			case AVMEDIA_TYPE_VIDEO:    info.MediaType = MediaType::Video;    break;
			case AVMEDIA_TYPE_AUDIO:    info.MediaType = MediaType::Audio;    break;
			case AVMEDIA_TYPE_SUBTITLE: info.MediaType = MediaType::Subtitle; break;
			default:                    info.MediaType = MediaType::Unknown;  break;
		}

		info.Index = index;
		info.Stream = mFile->streams[index];
		info.CodecParameters = info.Stream->codecpar;

		return info;
	}


	Core::Option<MediaStream*> MediaFile::GetStream(size_t index)
	{
		if (index >= mFile->nb_streams) return Core::NullOpt;

		if (!mOpenStreams.contains(index))
		{
			MediaStream stream(this, index);
			mOpenStreams.emplace(index, std::move(stream));
		}

		return &mOpenStreams.at(index);
	}


	Core::Option<MediaStream*> MediaFile::GetBestStream(MediaType type)
	{
		Core::Option<AVMediaType> streamTypeImpl;
		switch (type)
		{
			case MediaType::Audio:
				streamTypeImpl = AVMEDIA_TYPE_AUDIO; break;
			case MediaType::Video:
				streamTypeImpl = AVMEDIA_TYPE_VIDEO; break;
			case MediaType::Subtitle:
				streamTypeImpl = AVMEDIA_TYPE_SUBTITLE; break;
			case MediaType::Unknown:
			default:
				Core::DebugBreak(); return Core::NullOpt;
		}

		Core::Assert(streamTypeImpl.HasValue());
		auto index = av_find_best_stream(mFile, *streamTypeImpl, -1, -1, nullptr, 0);
		return (index >= 0) ? GetStream(index) : Core::NullOpt;
	}


	void MediaFile::Seek(size_t stream, size_t pts)
	{
		auto result = avformat_seek_file(mFile, static_cast<int>(stream), 0, static_cast<int>(pts), mFile->streams[stream]->duration, 0);
		Assert(result >= 0);
	}


	Core::Result<Packet, Core::IO::Error> MediaFile::Read()
	{
		Packet packet;
		auto result = av_read_frame(mFile, *packet);
		switch (result)
		{
			case 0: return packet;
			case AVERROR_EOF: return Core::IO::Error::EndOfFile;
			default: Core::DebugBreak(); return Core::IO::Error::Unknown;
		}
	}
}