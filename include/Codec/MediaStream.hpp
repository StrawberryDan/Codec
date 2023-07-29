#pragma once


#include "Strawberry/Core/Option.hpp"
#include "Strawberry/Core/Collection/CircularBuffer.hpp"
#include "Codec/Packet.hpp"
#include "Codec/Constants.hpp"
#include "Codec/Decoder.hpp"


namespace Strawberry::Codec
{
	class MediaFile;


	struct MediaStreamInfo
	{
		size_t             Index;
		MediaType          MediaType;
		AVCodecParameters* CodecParameters;
	};


	class MediaStream
	{
		friend class MediaFile;


	public:
		MediaStream(const MediaStream&)            = delete;
		MediaStream& operator=(const MediaStream&) = delete;
		MediaStream(MediaStream&&)                 = default;
		MediaStream& operator=(MediaStream&&)      = delete;


		Core::Option<Packet> Read();


		const AVCodec*           GetCodec() const;
		const AVCodecParameters* GetCodecParameters() const;


		Decoder GetDecoder() { return Decoder(GetCodec(), GetCodecParameters()); }


	private:
		MediaStream(MediaFile* file, size_t index);


	private:
		MediaStreamInfo                          mStreamInfo;
		MediaFile*                               mMediaFile = nullptr;
		bool                                     mIsEOF     = false;
		size_t                                   mNextPts   = 0;
		Core::Collection::CircularBuffer<Packet> mPacketBuffer;
	};
}