#pragma once

//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/Audio/Decoder.hpp"
#include "Codec/Constants.hpp"
#include "Codec/Packet.hpp"
// Core
#include "Strawberry/Core/Collection/CircularBuffer.hpp"
#include "Strawberry/Core/Math/Rational.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "Strawberry/Core/Types/ReflexivePointer.hpp"
// Standard Library
#include <chrono>


namespace Strawberry::Codec
{
	class MediaFile;


	struct MediaStreamInfo {
		size_t             Index;
		MediaType          MediaType;
		AVStream*          Stream;
		AVCodecParameters* CodecParameters;
	};


	class MediaStream
		: public Core::EnableReflexivePointer<MediaStream>
	{
		friend class MediaFile;


	public:
		MediaStream(const MediaStream&)            = delete;
		MediaStream& operator=(const MediaStream&) = delete;
		MediaStream(MediaStream&&) noexcept        = default;
		MediaStream& operator=(MediaStream&&)      = default;


		[[nodiscard]] Core::Optional<Packet> Read();


		[[nodiscard]] Core::Optional<std::string>   GetTitle() const;
		[[nodiscard]] Core::Optional<std::string>   GetAlbum() const;
		[[nodiscard]] Core::Optional<std::string>   GetArtist() const;
		[[nodiscard]] Core::Math::Rational<int64_t> GetTimeBase() const;
		[[nodiscard]] std::chrono::duration<double> GetDuration() const;
		[[nodiscard]] Core::Optional<size_t>        GetFrameCount() const;


		[[nodiscard]] const AVCodec*           GetCodec() const;
		[[nodiscard]] const AVCodecParameters* GetCodecParameters() const;


		[[nodiscard]] Audio::Decoder GetDecoder() const { return {GetCodec(), GetCodecParameters()}; }


	private:
		MediaStream(Core::ReflexivePointer<MediaFile> file, size_t index);


	private:
		MediaStreamInfo                          mStreamInfo;
		Core::ReflexivePointer<MediaFile>        mMediaFile = nullptr;
		bool                                     mIsEOF     = false;
		size_t                                   mNextPts   = 0;
		Core::Collection::CircularBuffer<Packet> mPacketBuffer;
		Core::Optional<int>                      mLastDTS;
	};
} // namespace Strawberry::Codec