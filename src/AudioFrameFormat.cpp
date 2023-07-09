#include "Codec/AudioFrameFormat.hpp"


#include "Codec/Frame.hpp"


namespace Strawberry::Codec
{
	AudioFrameFormat::AudioFrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout& inchannels,
									   int channelLayout)
			: sampleRate(sampleRate)
			  , sampleFormat(sampleFormat)
			  , channels{}
			  , channelLayout(channelLayout)
	{
		auto result = av_channel_layout_copy(&channels, &inchannels);
		Core::Assert(result == 0);
	}


	AudioFrameFormat::AudioFrameFormat(const Frame& frame)
			: sampleRate(frame->sample_rate)
			  , sampleFormat(frame->format)
			  , channels{}
			  , channelLayout(frame->channel_layout)
	{
		auto result = av_channel_layout_copy(&channels, &frame->ch_layout);
		Core::Assert(result == 0);
	}


	AudioFrameFormat::AudioFrameFormat(const AudioFrameFormat& rhs)
			: sampleRate(rhs.sampleRate)
			  , sampleFormat(rhs.sampleFormat)
			  , channels{}
			  , channelLayout(rhs.channelLayout)
	{
		auto result = av_channel_layout_copy(&channels, &rhs.channels);
		Core::Assert(result == 0);
	}


	AudioFrameFormat& AudioFrameFormat::operator=(const AudioFrameFormat& rhs)
	{
		if (this != &rhs)
		{
			std::construct_at(this, rhs);
		}

		return *this;
	}


	bool AudioFrameFormat::operator==(const AudioFrameFormat& b) const
	{
		return sampleRate == b.sampleRate
			   && sampleFormat == b.sampleFormat
			   && channels.nb_channels == b.channels.nb_channels
			   && channels.order == b.channels.order
			   && (channels.u.mask == b.channels.u.mask || channels.u.map == b.channels.u.map)
			   && channelLayout == b.channelLayout;
	}


	bool AudioFrameFormat::operator!=(const AudioFrameFormat& b) const
	{
		return !(*this == b);
	}
}