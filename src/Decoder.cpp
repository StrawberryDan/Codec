#include "Codec/Decoder.hpp"
#include "Core/Utilities.hpp"


#include "Core/Assert.hpp"



using Strawberry::Core::Assert;
using Strawberry::Core::Take;



namespace Strawberry::Codec
{
	Decoder::Decoder()
	    : mCodecContext(nullptr)
	    , mParameters{}
	{}



	Decoder::Decoder(const AVCodec* codec, const AVCodecParameters* parameters)
	    : mCodecContext(nullptr)
	    , mParameters(parameters)
	{
	    Assert(av_codec_is_decoder(codec));

	    mCodecContext = avcodec_alloc_context3(codec);
	    Assert(mCodecContext != nullptr);
	    auto result = avcodec_open2(mCodecContext, codec, nullptr);
	    Assert(result == 0);
	    Assert(avcodec_is_open(mCodecContext));
	    result = avcodec_parameters_to_context(mCodecContext, mParameters);
	    Assert(result >= 0);
	}



	Decoder::Decoder(Decoder&& other) noexcept
	    : mCodecContext(Take(other.mCodecContext))
	    , mParameters(Take(other.mParameters))
	{

	}



	Decoder& Decoder::operator=(Decoder&& other) noexcept
	{
	    mCodecContext = Take(other.mCodecContext);
	    mParameters = Take(other.mParameters);
	    return (*this);
	}



	Decoder::~Decoder()
	{
	    avcodec_close(mCodecContext);
	    avcodec_free_context(&mCodecContext);
	}



	std::vector<Frame> Decoder::DecodePacket(const Packet& packet)
	{
	    Assert(mCodecContext != nullptr);

	    auto sendResult = avcodec_send_packet(mCodecContext, *packet);
	    Assert(sendResult == 0 || sendResult == AVERROR(EAGAIN));

	    std::vector<Frame> frames;
	    int receiveResult;
	    do
	    {
	        Frame frame;
	        receiveResult = avcodec_receive_frame(mCodecContext, *frame);
	        Assert(receiveResult == 0 || receiveResult == AVERROR(EAGAIN) || receiveResult == AVERROR_EOF);
	        if (receiveResult == 0)
	        {
	            frames.push_back(std::move(frame));
	        }
	    } while (receiveResult == 0);

	    return frames;
	}
}