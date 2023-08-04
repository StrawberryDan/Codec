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
	Core::Option<Packet> MediaStream::Read()
	{
		if (mPacketBuffer.Empty() && !mIsEOF)
		{
			mMediaFile->Seek(mStreamInfo.Index, mNextPts);
			while (!mPacketBuffer.AtCapacity() && !mIsEOF)
			{
				auto packet = mMediaFile->Read();
				if (packet)
				{
					mNextPts = (*packet)->pts + (*packet)->duration;
					mPacketBuffer.Push(packet.Unwrap());
				}
				else switch (packet.Err())
				{
					case Core::IO::Error::EndOfFile: mIsEOF = true; break;
					default: Core::DebugBreak(); return Core::NullOpt;
				}
			}
		}

		return mPacketBuffer.Pop();
	}


	Core::Option<std::string> MediaStream::GetTitle() const
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


	Core::Option<std::string> MediaStream::GetAlbumTitle() const
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


	Core::Option<std::string> MediaStream::GetArtist() const
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


	Core::Math::Rational<> MediaStream::GetTimeBase() const
	{
		return
		{
			mStreamInfo.Stream->time_base.num,
			mStreamInfo.Stream->time_base.den
		};
	}


	std::chrono::duration<double> MediaStream::GetDuration() const
	{
		auto timeBase = GetTimeBase();
		auto timeBaseCount = mStreamInfo.Stream->duration;
		timeBase.Numerator() *= timeBaseCount;
		timeBase.Denominator() *= timeBaseCount;
		return std::chrono::duration<double>(timeBase.Evaluate());
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


	MediaStream::MediaStream(MediaFile* file, size_t index)
		: mStreamInfo(file->GetStreamInfo(index).Unwrap())
		, mMediaFile(file)
		, mPacketBuffer(256)
		, mNextPts(0)
	{}
}