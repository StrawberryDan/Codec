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
#include "Strawberry/Core/Util/Option.hpp"
// Standard Library
#include <chrono>


namespace Strawberry::Codec
{
	class MediaFile;


	struct MediaStreamInfo
	{
		size_t Index;
		MediaType MediaType;
		AVStream* Stream;
		AVCodecParameters* CodecParameters;
	};


	class MediaStream
	{
		friend class MediaFile;


	public:
		MediaStream(const MediaStream&) = delete;
		MediaStream& operator=(const MediaStream&) = delete;
		MediaStream(MediaStream&&) noexcept = default;
		MediaStream& operator=(MediaStream&&) = delete;


		[[nodiscard]] Core::Option<Packet> Read();


		[[nodiscard]] Core::Option<std::string> GetTitle() const;
		[[nodiscard]] Core::Option<std::string> GetAlbum() const;
		[[nodiscard]] Core::Option<std::string> GetArtist() const;
		[[nodiscard]] Core::Math::Rational<int64_t> GetTimeBase() const;
		[[nodiscard]] std::chrono::duration<double> GetDuration() const;
		[[nodiscard]] Core::Option<size_t> GetFrameCount() const;


		[[nodiscard]] const AVCodec* GetCodec() const;
		[[nodiscard]] const AVCodecParameters* GetCodecParameters() const;


		[[nodiscard]] Audio::Decoder GetDecoder() const { return {GetCodec(), GetCodecParameters()}; }


	private:
		MediaStream(MediaFile* file, size_t index);


	private:
		MediaStreamInfo mStreamInfo;
		MediaFile* mMediaFile = nullptr;
		bool mIsEOF = false;
		size_t mNextPts = 0;
		Core::Collection::CircularBuffer<Packet> mPacketBuffer;
		Core::Option<int> mLastDTS;
	};
}