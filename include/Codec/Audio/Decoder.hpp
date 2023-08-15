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
	class Decoder
	{
	public:
		Decoder(const AVCodec* codec, const AVCodecParameters* parameters);
		Decoder(const Decoder& other)            = delete;
		Decoder& operator=(const Decoder& other) = delete;
		Decoder(Decoder&& other) noexcept;
		Decoder& operator=(Decoder&& other) noexcept;
		~Decoder();


		void               Send(Packet packet);
		std::vector<Frame> Receive();


		[[nodiscard]] inline const AVCodecParameters* Parameters() const { return mParameters; }


	private:
		AVCodecContext*    mCodecContext;
		AVCodecParameters* mParameters;
		std::deque<Packet> mPacketBuffer;
	};
}// namespace Strawberry::Codec::Audio