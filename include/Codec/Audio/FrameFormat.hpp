#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Strawberry/Core/Util/Assert.hpp"


extern "C"
{
#include "libavutil/channel_layout.h"
}


namespace Strawberry::Codec::Audio
{
	class Frame;


	class FrameFormat
	{
	public:
		FrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout* channels);
		FrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout& channels);
		FrameFormat(const Frame& frame);

		FrameFormat(const FrameFormat& rhs);
		FrameFormat& operator=(const FrameFormat& rhs);
		FrameFormat(FrameFormat&& rhs) noexcept;
		FrameFormat& operator=(FrameFormat&& rhs) noexcept;


		bool operator==(const FrameFormat& b) const;
		bool operator!=(const FrameFormat& b) const;


		[[nodiscard]] int GetSampleRate() const;
		[[nodiscard]] int GetSampleFormat() const;
		[[nodiscard]] const AVChannelLayout* GetChannels() const;


	private:
		int mSampleRate;
		int mSampleFormat;
		AVChannelLayout mChannels;
	};
}
