#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Strawberry/Core/Assert.hpp"


extern "C"
{
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
}


namespace Strawberry::Codec::Audio
{
	class Frame;


	class FrameFormat
	{
	public:
		FrameFormat(int sampleRate, AVSampleFormat sampleFormat, const AVChannelLayout* channels);
		FrameFormat(int sampleRate, AVSampleFormat sampleFormat, const AVChannelLayout& channels);
		FrameFormat(const Frame& frame);

		FrameFormat(const FrameFormat& rhs);
		FrameFormat& operator=(const FrameFormat& rhs);
		FrameFormat(FrameFormat&& rhs) noexcept;
		FrameFormat& operator=(FrameFormat&& rhs) noexcept;


		bool operator==(const FrameFormat& b) const;
		bool operator!=(const FrameFormat& b) const;


		[[nodiscard]] int                    GetSampleRate() const;
		[[nodiscard]] AVSampleFormat         GetSampleFormat() const;
		[[nodiscard]] const AVChannelLayout* GetChannels() const;
		[[nodiscard]] int                    GetSizeInBytes() const;

	private:
		int             mSampleRate;
		AVSampleFormat  mSampleFormat;
		AVChannelLayout mChannels;
	};
} // namespace Strawberry::Codec::Audio
