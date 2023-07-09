#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Strawberry/Core/Assert.hpp"


extern "C"
{
#include <libavutil/channel_layout.h>
}


namespace Strawberry::Codec
{
	class Frame;


	struct AudioFrameFormat
	{
		int sampleRate;
		int sampleFormat;
		AVChannelLayout channels;
		int channelLayout = 0;


		explicit AudioFrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout& inchannels, int channelLayout = 0);

		explicit AudioFrameFormat(const Frame& frame);


		AudioFrameFormat(const AudioFrameFormat& rhs);


		AudioFrameFormat& operator=(const AudioFrameFormat& rhs);


		bool operator==(const AudioFrameFormat& b) const;


		bool operator!=(const AudioFrameFormat& b) const;
	};
}
