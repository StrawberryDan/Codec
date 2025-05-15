#pragma once

//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/Audio/AudioDecoder.hpp"
#include "Codec/Constants.hpp"
#include "Codec/Packet.hpp"
// Core
#include "Strawberry/Core/Math/Rational.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "Strawberry/Core/Types/ReflexivePointer.hpp"
#include "Strawberry/Core/Timing/Clock.hpp"
// Standard Library
#include <chrono>
#include <deque>

#include "Error.hpp"


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
			: public Core::EnableReflexivePointer
	{
		friend class MediaFile;

	public:
		MediaStream(const MediaStream&)            = delete;
		MediaStream& operator=(const MediaStream&) = delete;
		MediaStream(MediaStream&&) noexcept        = default;
		MediaStream& operator=(MediaStream&&)      = default;


		[[nodiscard]] Core::Result<Packet, Error>   Read();
		void                                        Seek(Core::Seconds time);


		[[nodiscard]] Core::Optional<std::string>   GetTitle() const;
		[[nodiscard]] Core::Optional<std::string>   GetAlbum() const;
		[[nodiscard]] Core::Optional<std::string>   GetArtist() const;
		[[nodiscard]] Core::Math::Rational<int64_t> GetTimeBase() const;
		[[nodiscard]] Core::Seconds                 GetDuration() const;


		[[nodiscard]] const AVCodec*           GetCodec() const;
		[[nodiscard]] const AVCodecParameters* GetCodecParameters() const;


		[[nodiscard]] Audio::AudioDecoder GetDecoder() const
		{
			return {GetCodec(), GetCodecParameters()};
		}

	private:
		MediaStream(const MediaFile& file, size_t index);

	private:
		MediaStreamInfo                          mStreamInfo;
		AVFormatContext*                         mFile;
		bool                                     mIsEOF     = false;
	};
} // namespace Strawberry::Codec
