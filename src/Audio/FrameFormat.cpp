#include "Codec/Audio/FrameFormat.hpp"


#include "Codec/Audio/Frame.hpp"


namespace Strawberry::Codec::Audio
{
	FrameFormat::FrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout* channels)
		: mSampleRate(sampleRate)
		, mSampleFormat(sampleFormat)
		, mChannels{}
	{
		auto result = av_channel_layout_copy(&mChannels, channels);
		Core::Assert(result == 0);

		Core::Assert(sampleRate > 0);
		Core::Assert(channels->nb_channels > 0);
	}


	FrameFormat::FrameFormat(int sampleRate, int sampleFormat, const AVChannelLayout& channels)
		: FrameFormat(sampleRate, sampleFormat, &channels)
	{}


	FrameFormat::FrameFormat(const Frame& frame)
		: mSampleRate(frame->sample_rate)
		, mSampleFormat(frame->format)
		, mChannels{}
	{
		auto result = av_channel_layout_copy(&mChannels, &frame->ch_layout);
		Core::Assert(result == 0);
	}


	FrameFormat::FrameFormat(const FrameFormat& rhs)
		: mSampleRate(rhs.mSampleRate)
		, mSampleFormat(rhs.mSampleFormat)
		, mChannels{}
	{
		auto result = av_channel_layout_copy(&mChannels, &rhs.mChannels);
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


	FrameFormat::FrameFormat(FrameFormat&& rhs) noexcept
		: mSampleRate(rhs.mSampleRate)
		, mSampleFormat(rhs.mSampleFormat)
		, mChannels{}
	{
		auto result = av_channel_layout_copy(&mChannels, &rhs.mChannels);
		Core::Assert(result == 0);
	}


	FrameFormat& FrameFormat::operator=(FrameFormat&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	bool FrameFormat::operator==(const FrameFormat& b) const
	{
		return mSampleRate == b.mSampleRate && mSampleFormat == b.mSampleFormat && mChannels.nb_channels == b.mChannels.nb_channels && mChannels.order == b.mChannels.order && (mChannels.u.mask == b.mChannels.u.mask || mChannels.u.map == b.mChannels.u.map);
	}


	bool FrameFormat::operator!=(const FrameFormat& b) const
	{
		return !(*this == b);
	}


	int FrameFormat::GetSampleRate() const
	{
		return mSampleRate;
	}


	int FrameFormat::GetSampleFormat() const
	{
		return mSampleFormat;
	}


	const AVChannelLayout* FrameFormat::GetChannels() const
	{
		return &mChannels;
	}
}// namespace Strawberry::Codec::Audio