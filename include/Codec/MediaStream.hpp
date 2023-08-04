#pragma once

//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/Packet.hpp"
#include "Codec/Constants.hpp"
#include "Codec/Audio/Decoder.hpp"
// Core
#include "Strawberry/Core/Collection/CircularBuffer.hpp"
#include "Strawberry/Core/Math/Rational.hpp"
#include "Strawberry/Core/Option.hpp"
// Standard Library
#include <chrono>



namespace Strawberry::Codec
{
	class MediaFile;


	struct MediaStreamInfo
	{
		size_t             Index;
		MediaType          MediaType;
		AVStream*          Stream;
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


		Core::Option<std::string>     GetTitle()      const;
		Core::Option<std::string>     GetAlbumTitle() const;
		Core::Option<std::string>     GetArtist()     const;
		Core::Math::Rational<>        GetTimeBase()   const;
		std::chrono::duration<double> GetDuration()   const;


		const AVCodec*           GetCodec() const;
		const AVCodecParameters* GetCodecParameters() const;


		Audio::Decoder GetDecoder() { return Audio::Decoder(GetCodec(), GetCodecParameters()); }


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