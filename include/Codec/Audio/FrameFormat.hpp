#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Strawberry/Core/Assert.hpp"


extern "C"
{
#include <libavutil/channel_layout.h>
}


namespace Strawberry::Codec::Audio
{
	class Frame;


	struct FrameFormat
	{
		int sampleRate;
		int sampleFormat;
		AVChannelLayout channels;
		int channelLayout = 0;


		explicit FrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout& inchannels, int channelLayout = 0);

		explicit FrameFormat(const Frame& frame);


		FrameFormat(const FrameFormat& rhs);


		FrameFormat& operator=(const FrameFormat& rhs);


		bool operator==(const FrameFormat& b) const;


		bool operator!=(const FrameFormat& b) const;
	};
}