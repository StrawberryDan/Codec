#pragma once



#include "Codec/Packet.hpp"
#include "Codec/Audio/Frame.hpp"
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
	    Decoder(const Decoder& other) = delete;
	    Decoder& operator=(const Decoder& other) = delete;
	    Decoder(Decoder&& other) noexcept ;
	    Decoder& operator=(Decoder&& other) noexcept ;
	    ~Decoder();


	    std::vector<Frame> DecodePacket(const Packet& packet);


	    [[nodiscard]] inline const AVCodecParameters* Parameters() const { return mParameters; }

	private:
	    AVCodecContext* mCodecContext;
	    const AVCodecParameters* mParameters;
	};
}