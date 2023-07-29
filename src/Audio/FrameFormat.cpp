#include "Codec/Audio/FrameFormat.hpp"


#include "Codec/Audio/Frame.hpp"


namespace Strawberry::Codec::Audio
{
	FrameFormat::FrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout& inchannels,
									   int channelLayout)
			: sampleRate(sampleRate)
			  , sampleFormat(sampleFormat)
			  , channels{}
			  , channelLayout(channelLayout)
	{
		auto result = av_channel_layout_copy(&channels, &inchannels);
		Core::Assert(result == 0);
	}


	FrameFormat::FrameFormat(const Frame& frame)
			: sampleRate(frame->sample_rate)
			  , sampleFormat(frame->format)
			  , channels{}
			  , channelLayout(frame->channel_layout)
	{
		auto result = av_channel_layout_copy(&channels, &frame->ch_layout);
		Core::Assert(result == 0);
	}


	FrameFormat::FrameFormat(const FrameFormat& rhs)
			: sampleRate(rhs.sampleRate)
			  , sampleFormat(rhs.sampleFormat)
			  , channels{}
			  , channelLayout(rhs.channelLayout)
	{
		auto result = av_channel_layout_copy(&channels, &rhs.channels);
		Core::Assert(result == 0);
	}


	FrameFormat& FrameFormat::operator=(const FrameFormat& rhs)
	{
		if (this != &rhs)
		{
			std::construct_at(this, rhs);
		}

		return *this;
	}


	bool FrameFormat::operator==(const FrameFormat& b) const
	{
		return sampleRate == b.sampleRate
			   && sampleFormat == b.sampleFormat
			   && channels.nb_channels == b.channels.nb_channels
			   && channels.order == b.channels.order
			   && (channels.u.mask == b.channels.u.mask || channels.u.map == b.channels.u.map)
			   && (channelLayout == 0 || b.channelLayout == 0 || channelLayout == b.channelLayout);
	}


	bool FrameFormat::operator!=(const FrameFormat& b) const
	{
		return !(*this == b);
	}
}