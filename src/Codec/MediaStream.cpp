//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/MediaStream.hpp"
// Codec
#include "Codec/MediaFile.hpp"

// LibAV
extern "C"
{
#include "libavutil/dict.h"
}


namespace Strawberry::Codec
{
	Core::Result<Packet, Error> MediaStream::Read()
	{
		Packet packet;
		auto   result = av_read_frame(mFile, *packet);
		switch (result)
		{
		case 0:
			return packet;
		case AVERROR_EOF:
			return ErrorEndOfFile();
		default:
			Core::Unreachable();
		}
	}


	void MediaStream::Seek(double time)
	{
		int  ts     = time * static_cast<double>(mStreamInfo.Stream->time_base.den) / static_cast<double>(mStreamInfo.Stream->time_base.num);
		auto result = avformat_seek_file(mFile, mStreamInfo.Index, 0, ts, ts, AVSEEK_FLAG_FRAME);
		Core::Assert(result >= 0);
		result = avformat_flush(mFile);
		Core::Assert(result >= 0);
		mIsEOF = time > GetDuration();
	}


	Core::Optional<std::string> MediaStream::GetTitle() const
	{
		auto entry = av_dict_get(mStreamInfo.Stream->metadata, "title", nullptr, 0);
		if (entry)
		{
			return std::string(entry->value);
		}
		else
		{
			return Core::NullOpt;
		}
	}


	Core::Optional<std::string> MediaStream::GetAlbum() const
	{
		auto entry = av_dict_get(mStreamInfo.Stream->metadata, "album", nullptr, 0);
		if (entry)
		{
			return std::string(entry->value);
		}
		else
		{
			return Core::NullOpt;
		}
	}


	Core::Optional<std::string> MediaStream::GetArtist() const
	{
		auto entry = av_dict_get(mStreamInfo.Stream->metadata, "artist", nullptr, 0);
		if (entry)
		{
			return std::string(entry->value);
		}
		else
		{
			return Core::NullOpt;
		}
	}


	Core::Math::Rational<int64_t> MediaStream::GetTimeBase() const
	{
		return {mStreamInfo.Stream->time_base.num, mStreamInfo.Stream->time_base.den};
	}


	double MediaStream::GetDuration() const
	{
		auto timeBase      = GetTimeBase();
		auto timeBaseCount = mStreamInfo.Stream->duration;
		timeBase           = timeBase * timeBaseCount;
		return timeBase.Evaluate();
	}


	const AVCodec* MediaStream::GetCodec() const
	{
		auto codec = avcodec_find_decoder(mStreamInfo.CodecParameters->codec_id);
		Core::Assert(codec);
		return codec;
	}


	const AVCodecParameters* MediaStream::GetCodecParameters() const
	{
		Core::Assert(mStreamInfo.CodecParameters);
		return mStreamInfo.CodecParameters;
	}

	MediaStream::MediaStream(const MediaFile& file, size_t index)
		: mStreamInfo(file.GetStreamInfo(index))
		, mFile(nullptr)
	{
		auto result = avformat_open_input(&mFile, file.GetPath().string().c_str(), nullptr, nullptr);
		Core::AssertEQ(result, 0);

		result = avformat_find_stream_info(mFile, nullptr);
		Core::Assert(result >= 0);

		result = av_seek_frame(mFile, mStreamInfo.Index, -1, AVSEEK_FLAG_BACKWARD);
		Core::Assert(result >= 0);
	}
} // namespace Strawberry::Codec
