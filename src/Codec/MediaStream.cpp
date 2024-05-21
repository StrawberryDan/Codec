//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/MediaStream.hpp"
// Codec
#include "Codec/MediaFile.hpp"
// LibAV
extern "C" {
#include "libavutil/dict.h"
}


namespace Strawberry::Codec
{
	Core::Optional<Packet> MediaStream::Read()
	{
		if (mPacketBuffer.Empty() && !mIsEOF)
		{
			while (!mPacketBuffer.AtCapacity() && !mIsEOF)
			{
				auto packet = mMediaFile->Read();
				if (packet)
				{
					if ((*packet)->stream_index != mStreamInfo.Index) { continue; }
					mPacketBuffer.Push(packet.Unwrap());
				}
				else
					switch (packet.Err())
					{
						case Core::IO::Error::EndOfFile:
							mIsEOF = true;
							break;
						default:
							Core::DebugBreak();
							return Core::NullOpt;
					}
			}
		}

		return mPacketBuffer.Pop();
	}


	void MediaStream::Seek(Core::Seconds time)
	{
		int ts = time * static_cast<double>(mStreamInfo.Stream->time_base.den) / static_cast<double>(mStreamInfo.Stream->time_base.num);
		auto result = avformat_seek_file(mMediaFile->mFile, mStreamInfo.Index, 0, ts, ts, AVSEEK_FLAG_FRAME);
		Core::Assert(result >= 0);
		result = avformat_flush(mMediaFile->mFile);
		Core::Assert(result >= 0);
		mIsEOF = time > GetDuration();
		mPacketBuffer.Clear();
	}


	Core::Optional<std::string> MediaStream::GetTitle() const
	{
		auto entry = av_dict_get(mStreamInfo.Stream->metadata, "title", nullptr, 0);
		if (entry) { return std::string(entry->value); }
		else { return Core::NullOpt; }
	}

	Core::Optional<std::string> MediaStream::GetAlbum() const
	{
		auto entry = av_dict_get(mStreamInfo.Stream->metadata, "album", nullptr, 0);
		if (entry) { return std::string(entry->value); }
		else { return Core::NullOpt; }
	}

	Core::Optional<std::string> MediaStream::GetArtist() const
	{
		auto entry = av_dict_get(mStreamInfo.Stream->metadata, "artist", nullptr, 0);
		if (entry) { return std::string(entry->value); }
		else { return Core::NullOpt; }
	}


	Core::Math::Rational<int64_t> MediaStream::GetTimeBase() const
	{
		return {mStreamInfo.Stream->time_base.num, mStreamInfo.Stream->time_base.den};
	}


	Core::Seconds MediaStream::GetDuration() const
	{
		auto timeBase      = GetTimeBase();
		auto timeBaseCount = mStreamInfo.Stream->duration;
		timeBase           = timeBase * timeBaseCount;
		return timeBase.Evaluate();
	}

	Core::Optional<size_t> MediaStream::GetFrameCount() const
	{
		return mPacketBuffer.Size();
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


	MediaStream::MediaStream(Core::ReflexivePointer<MediaFile> file, size_t index)
		: mStreamInfo(file->GetStreamInfo(index).Unwrap())
		, mMediaFile(file)
		, mPacketBuffer(256)
	{}
} // namespace Strawberry::Codec