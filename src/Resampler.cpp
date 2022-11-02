#include "Codec/Resampler.hpp"
#include "Standard/Utilities.hpp"


#include "Standard/Assert.hpp"



using Strawberry::Standard::Assert;
using Strawberry::Standard::Take;



static constexpr unsigned int    TARGET_SAMPLE_RATE    = 48000;
static constexpr AVChannelLayout TARGET_CHANNEL_LAYOUT = AV_CHANNEL_LAYOUT_STEREO;
static constexpr AVSampleFormat  TARGET_SAMPLE_FORMAT  = AV_SAMPLE_FMT_FLTP;



namespace Strawberry::Codec
{
	Resampler::Resampler()
	    :mSwrContext(nullptr)
	{

	}



	Resampler::Resampler(const AVCodecParameters* codecParameters)
	    : mSwrContext(nullptr)
	{
	    auto target_channel_layout = TARGET_CHANNEL_LAYOUT;
	    auto source_channel_layout = codecParameters->ch_layout;
	    auto result = swr_alloc_set_opts2(&mSwrContext, &target_channel_layout, TARGET_SAMPLE_FORMAT, TARGET_SAMPLE_RATE, &source_channel_layout, static_cast<AVSampleFormat>(codecParameters->format), codecParameters->sample_rate, 0, nullptr);
	    Assert(result == 0);
	}



	Resampler::Resampler(Resampler&& other) noexcept
	    : mSwrContext(Take(other.mSwrContext))
	{

	}



	Resampler& Resampler::operator=(Resampler&& other) noexcept
	{
	    mSwrContext = Take(other.mSwrContext);
	    return (*this);
	}



	Resampler::~Resampler()
	{
	    swr_free(&mSwrContext);
	}



	Frame Resampler::Resample(const Frame& input)
	{
	    Assert(mSwrContext != nullptr);

	    Frame output;
	    output->ch_layout = TARGET_CHANNEL_LAYOUT;
	    output->format = TARGET_SAMPLE_FORMAT;
	    output->sample_rate = TARGET_SAMPLE_RATE;
	    auto result = swr_convert_frame(mSwrContext, *output, *input);
	    Assert(result == 0);

	    return output;
	}



	std::vector<Frame> Resampler::Resample(const std::vector<Frame>& input)
	{
	    Assert(mSwrContext != nullptr);
	    std::vector<Frame> output;
	    output.reserve(input.size());
	    for (const auto& frame : input)
	    {
	        output.push_back(Resample(frame));
	    }
	    return output;
	}
}