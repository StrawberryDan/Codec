#include "Codec/Audio/AudioEncoder.hpp"


#include "Codec/Packet.hpp"
#include "Strawberry/Core/Assert.hpp"


extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


namespace Strawberry::Codec::Audio
{
	AudioEncoder::AudioEncoder(AVCodecID codecID, AVChannelLayout channelLayout)
		: mContext(nullptr)
		, mParameters(nullptr)
	{
		const AVCodec* codec = avcodec_find_encoder(codecID);
		Core::Assert(codec != nullptr);
		Core::Assert(av_codec_is_encoder(codec));
		mContext = avcodec_alloc_context3(codec);
		Core::Assert(mContext != nullptr);
		mContext->strict_std_compliance = 0;

		const void* frameRates;
		int numFramerates = 0;
		avcodec_get_supported_config(mContext, codec, AV_CODEC_CONFIG_SAMPLE_RATE, 0, &frameRates, &numFramerates);

		const void* sampleFormats;
		int numSampleFormats = 0;
		avcodec_get_supported_config(mContext, codec, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0, &sampleFormats, &numSampleFormats);

		mContext->sample_rate           = static_cast<const int*>(frameRates)[0];
		mContext->time_base             = AVRational{1, mContext->sample_rate};
		mContext->sample_fmt            = static_cast<const AVSampleFormat*>(sampleFormats)[0];
		mContext->ch_layout             = channelLayout;

		auto result = avcodec_open2(mContext, codec, nullptr);
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


	AudioEncoder::AudioEncoder(AudioEncoder&& rhs)
		: mContext(std::exchange(rhs.mContext, nullptr))
		, mParameters(std::exchange(rhs.mParameters, nullptr))
		, mFrameResampler(std::move(rhs.mFrameResampler))
		, mFrameResizer(std::move(rhs.mFrameResizer))
		, mFrameBuffer(std::move(rhs.mFrameBuffer)) {}


	AudioEncoder::~AudioEncoder()
	{
		if (mContext && mParameters)
		{
			avcodec_parameters_free(&mParameters);
			avcodec_free_context(&mContext);
		}
	}


	void AudioEncoder::Send(Frame frame)
	{
		mFrameBuffer.push_back(std::move(frame));
	}


	std::vector<Packet> AudioEncoder::Receive()
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
				do
				{
					Packet packet;
					receive = avcodec_receive_packet(mContext, *packet);
					Core::Assert(receive == 0 || receive == AVERROR(EAGAIN));

					if (receive == 0)
					{
						packets.push_back(packet);
					}
				}
				while (receive == 0);
			}
		}

		return packets;
	}


	std::vector<Packet> AudioEncoder::Flush()
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
				do
				{
					Packet packet;
					receive = avcodec_receive_packet(mContext, *packet);
					Core::Assert(receive == 0 || receive == AVERROR(EAGAIN));

					if (receive == 0)
					{
						packets.push_back(packet);
					}
				}
				while (receive == 0);
			}
		}

		return packets;
	}


	AVCodecParameters* AudioEncoder::Parameters() const
	{
		return mParameters;
	}
} // namespace Strawberry::Codec::Audio
