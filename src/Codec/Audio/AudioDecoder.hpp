#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/Audio/Frame.hpp"
#include "Codec/Packet.hpp"
// Standard Library
#include <deque>
#include <vector>


extern "C"
{
#include "libavcodec/avcodec.h"
}


namespace Strawberry::Codec::Audio
{
	class AudioDecoder
	{
	public:
		AudioDecoder(const AVCodec* codec, const AVCodecParameters* parameters);
		AudioDecoder(const AudioDecoder& other)            = delete;
		AudioDecoder& operator=(const AudioDecoder& other) = delete;
		AudioDecoder(AudioDecoder&& other) noexcept;
		AudioDecoder& operator=(AudioDecoder&& other) noexcept;
		~AudioDecoder();


		void               Send(Packet packet);
		std::vector<Frame> Receive();


		[[nodiscard]] inline const AVCodecParameters* Parameters() const
		{
			return mParameters;
		}

	private:
		AVCodecContext*    mCodecContext;
		AVCodecParameters* mParameters;
		std::deque<Packet> mPacketBuffer;
	};
} // namespace Strawberry::Codec::Audio
