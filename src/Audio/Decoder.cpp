#include "Codec/Audio/Decoder.hpp"
#include "Strawberry/Core/Util/Utilities.hpp"


#include "Strawberry/Core/Util/Assert.hpp"
#include "Strawberry/Core/Util/Option.hpp"



using Strawberry::Core::Assert;
using Strawberry::Core::Take;



namespace Strawberry::Codec::Audio
{
	Decoder::Decoder(const AVCodec* codec, const AVCodecParameters* parameters)
		: mCodecContext(nullptr)
		, mParameters(nullptr)
	{
		Assert(av_codec_is_decoder(codec));


		mParameters = avcodec_parameters_alloc();
		Assert(mParameters);
		auto result = avcodec_parameters_copy(mParameters, parameters);
		Assert(result >= 0);


		mCodecContext = avcodec_alloc_context3(codec);
		Assert(mCodecContext != nullptr);

		// Set channel layout to stereo if none is already specified.
		if (mCodecContext->codec_type == AVMEDIA_TYPE_AUDIO && (mCodecContext->ch_layout.nb_channels == 0))
		{
			mCodecContext->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
		}

		result = avcodec_open2(mCodecContext, codec, nullptr);
		Assert(result == 0);
		Assert(avcodec_is_open(mCodecContext));
		result = avcodec_parameters_to_context(mCodecContext, mParameters);
		Assert(result >= 0);
	}



	Decoder::Decoder(Decoder&& other) noexcept
		: mCodecContext(std::exchange(other.mCodecContext, nullptr))
		, mParameters(std::exchange(other.mParameters, nullptr))
	{

	}



	Decoder& Decoder::operator=(Decoder&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return *this;
	}



	Decoder::~Decoder()
	{
		if (mParameters)
		{
			avcodec_parameters_free(&mParameters);
		}

		if (mCodecContext)
		{
			avcodec_close(mCodecContext);
			avcodec_free_context(&mCodecContext);
		}
	}


	void Decoder::Send(Packet packet)
	{
		mPacketBuffer.push_back(std::move(packet));
	}


	std::vector<Frame> Decoder::Receive()
	{
		if (mPacketBuffer.empty())
			return {};


		Packet packet(std::move(mPacketBuffer.front()));
		mPacketBuffer.pop_front();


		Core::Assert(mCodecContext != nullptr);
		auto sendResult = avcodec_send_packet(mCodecContext, *packet);
		Assert(sendResult == 0 || sendResult == AVERROR(EAGAIN));

		std::vector<Frame> frames;
		int receiveResult;
		do
		{
			Frame frame = Frame::Allocate();
			receiveResult = avcodec_receive_frame(mCodecContext, *frame);
			Assert(receiveResult == 0 || receiveResult == AVERROR(EAGAIN) || receiveResult == AVERROR_EOF);
			if (receiveResult == 0)
			{
				frames.push_back(std::move(frame));
			}
		}
		while (receiveResult == 0);

		return frames;
	}
}