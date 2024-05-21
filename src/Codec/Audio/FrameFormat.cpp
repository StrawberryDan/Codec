#include "Codec/Audio/FrameFormat.hpp"


#include "Codec/Audio/Frame.hpp"


namespace Strawberry::Codec::Audio
{
	FrameFormat::FrameFormat(int sampleRate, AVSampleFormat sampleFormat, const AVChannelLayout* channels)
		: mSampleRate(sampleRate)
		, mSampleFormat(sampleFormat)
		, mChannels{}
	{
		auto result = av_channel_layout_copy(&mChannels, channels);
		Core::Assert(result == 0);

		Core::Assert(sampleRate > 0);
		Core::Assert(channels->nb_channels > 0);
	}


	FrameFormat::FrameFormat(int sampleRate, AVSampleFormat sampleFormat, const AVChannelLayout& channels)
		: FrameFormat(sampleRate, sampleFormat, &channels)
	{}


	FrameFormat::FrameFormat(const Frame& frame)
		: mSampleRate(frame->sample_rate)
		, mSampleFormat(static_cast<AVSampleFormat>(frame->format))
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
		if (this != &rhs) { std::construct_at(this, rhs); }

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
		if (this != &rhs) { std::construct_at(this, std::move(rhs)); }

		return *this;
	}


	bool FrameFormat::operator==(const FrameFormat& b) const
	{
		return mSampleRate == b.mSampleRate && mSampleFormat == b.mSampleFormat && mChannels.nb_channels == b.mChannels.nb_channels &&
			   mChannels.order == b.mChannels.order && (mChannels.u.mask == b.mChannels.u.mask || mChannels.u.map == b.mChannels.u.map);
	}


	bool FrameFormat::operator!=(const FrameFormat& b) const
	{
		return !(*this == b);
	}


	int FrameFormat::GetSampleRate() const
	{
		return mSampleRate;
	}


	AVSampleFormat FrameFormat::GetSampleFormat() const
	{
		return static_cast<AVSampleFormat>(mSampleFormat);
	}


	const AVChannelLayout* FrameFormat::GetChannels() const
	{
		return &mChannels;
	}


	int FrameFormat::GetSizeInBytes() const
	{
		return av_get_bytes_per_sample(static_cast<AVSampleFormat>(mSampleFormat));
	}
} // namespace Strawberry::Codec::Audio