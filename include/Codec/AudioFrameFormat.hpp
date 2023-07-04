#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include <libavutil/channel_layout.h>
#include "Strawberry/Core/Assert.hpp"


namespace Strawberry::Codec
{
	struct AudioFrameFormat
	{
		int sampleRate;
		int sampleFormat;
		AVChannelLayout channels;
		int channelLayout = 0;


		explicit AudioFrameFormat(const Frame& frame)
			: sampleRate(frame->sample_rate)
			, sampleFormat(frame->format)
			, channels{}
			, channelLayout(frame->channel_layout)
		{
			auto result = av_channel_layout_copy(&channels, &frame->ch_layout);
			Core::Assert(result == 0);
		}


		AudioFrameFormat(const AudioFrameFormat& rhs)
				: sampleRate(rhs.sampleRate)
				  , sampleFormat(rhs.sampleFormat)
				  , channels{}
				  , channelLayout(rhs.channelLayout)
		{
			auto result = av_channel_layout_copy(&channels, &rhs.channels);
			Core::Assert(result == 0);
		}


		AudioFrameFormat& operator=(const AudioFrameFormat& rhs)
		{
			if (this != &rhs)
			{
				std::construct_at(this, rhs);
			}

			return *this;
		}
	};
}


bool operator==(const Strawberry::Codec::AudioFrameFormat& a, const Strawberry::Codec::AudioFrameFormat& b)
{
	return a.sampleRate == b.sampleRate
		   && a.sampleFormat == b.sampleFormat
		   && a.channels.nb_channels == b.channels.nb_channels
		   && a.channels.order == b.channels.order
		   && (a.channels.u.mask == b.channels.u.mask || a.channels.u.map == b.channels.u.map)
		   && a.channelLayout == b.channelLayout;
}


bool operator!=(const Strawberry::Codec::AudioFrameFormat& a, const Strawberry::Codec::AudioFrameFormat& b)
{
	return !(a == b);
}
