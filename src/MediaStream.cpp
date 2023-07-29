#include "Codec/MediaStream.hpp"


#include "Codec/MediaFile.hpp"


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