#include "Codec/Audio/Encoder.hpp"


#include "Codec/Packet.hpp"
#include "Strawberry/Core/Util/Assert.hpp"


extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


namespace Strawberry::Codec::Audio
{
	Encoder::Encoder(AVCodecID codecID, AVChannelLayout channelLayout)
		: mContext(nullptr)
		, mParameters(nullptr)
	{
		const AVCodec* codec = avcodec_find_encoder(codecID);
		Core::Assert(codec != nullptr);
		Core::Assert(av_codec_is_encoder(codec));
		mContext = avcodec_alloc_context3(codec);
		Core::Assert(mContext != nullptr);
		mContext->strict_std_compliance = -2;
		mContext->sample_rate           = codec->supported_samplerates[0];
		mContext->time_base             = AVRational{1, mContext->sample_rate};
		mContext->sample_fmt            = codec->sample_fmts[0];
		mContext->ch_layout             = channelLayout;

		auto result                     = avcodec_open2(mContext, codec, nullptr);
		Core::Assert(result == 0);
		Core::Assert(avcodec_is_open(mContext));

		mParameters = avcodec_parameters_alloc();
		Core::Assert(mParameters != nullptr);
		result = avcodec_parameters_from_context(mParameters, mContext);
		Core::Assert(result >= 0);


		FrameFormat format(mContext->sample_rate, mContext->sample_fmt, &mContext->ch_layout);
		mFrameResampler.Emplace(format);
		mFrameResizer.Emplace(mContext->frame_size);
	}


	Encoder::~Encoder()
	{
		avcodec_parameters_free(&mParameters);
		avcodec_close(mContext);
		avcodec_free_context(&mContext);
	}


	void Encoder::Send(Frame frame)
	{
		mFrameBuffer.push_back(std::move(frame));
	}


	std::vector<Packet> Encoder::Receive()
	{
		if (mFrameBuffer.empty()) return {};

		Frame frame = std::move(mFrameBuffer.front());
		mFrameBuffer.pop_back();

		std::vector<Packet> packets;
		mFrameResizer->SendFrame(std::move(frame));
		while (auto frame = mFrameResizer->ReadFrame(FrameResizer::Mode::WaitForFullFrames))
		{
			mFrameResampler->SendFrame(frame.Unwrap());
			while (auto frame = mFrameResampler->ReadFrame())
			{
				auto send = avcodec_send_frame(mContext, **frame);
				Core::Assert(send == 0);

				int receive;
				do {
					Packet packet;
					receive = avcodec_receive_packet(mContext, *packet);
					Core::Assert(receive == 0 || receive == AVERROR(EAGAIN));

					if (receive == 0) { packets.push_back(packet); }
				} while (receive == 0);
			}
		}

		return packets;
	}


	std::vector<Packet> Encoder::Flush()
	{
		std::vector<Packet> packets;

		while (auto frame = mFrameResizer->ReadFrame(FrameResizer::Mode::YieldAvailable))
		{
			mFrameResampler->SendFrame(frame.Unwrap());
			while (auto frame = mFrameResampler->ReadFrame())
			{
				auto send = avcodec_send_frame(mContext, **frame);
				Core::Assert(send == 0);

				int receive;
				do {
					Packet packet;
					receive = avcodec_receive_packet(mContext, *packet);
					Core::Assert(receive == 0 || receive == AVERROR(EAGAIN));

					if (receive == 0) { packets.push_back(packet); }
				} while (receive == 0);
			}
		}

		return packets;
	}


	AVCodecParameters* Encoder::Parameters() const
	{
		return mParameters;
	}
} // namespace Strawberry::Codec::Audio