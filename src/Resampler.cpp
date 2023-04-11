#include "Codec/Resampler.hpp"
#include "Core/Utilities.hpp"


#include "Core/Assert.hpp"



using Strawberry::Core::Assert;
using Strawberry::Core::Take;



static constexpr unsigned int    TARGET_SAMPLE_RATE    = 48000;
static constexpr AVChannelLayout TARGET_CHANNEL_LAYOUT = AV_CHANNEL_LAYOUT_STEREO;
static constexpr AVSampleFormat  TARGET_SAMPLE_FORMAT  = AV_SAMPLE_FMT_FLTP;



namespace Strawberry::Codec
{
	Resampler::Resampler(unsigned int targetSampleRate, AVChannelLayout targetLayout, AVSampleFormat targetFormat, const AVCodecParameters* codecParameters)
		: mSwrContext(nullptr)
	{
		auto source_channel_layout = codecParameters->ch_layout;
		auto result = swr_alloc_set_opts2(
				&mSwrContext,
				&targetLayout,
				targetFormat,
				targetSampleRate,
				&codecParameters->ch_layout,
				static_cast<AVSampleFormat>(codecParameters->format),
				codecParameters->sample_rate,
				0,
				nullptr);
		Assert(result == 0);
	}



	Resampler::Resampler(Resampler&& other) noexcept
		: mSwrContext(Take(other.mSwrContext))
	{

	}



	Resampler& Resampler::operator=(Resampler&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

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
		output->ch_layout	= TARGET_CHANNEL_LAYOUT;
		output->format		= TARGET_SAMPLE_FORMAT;
		output->sample_rate	= TARGET_SAMPLE_RATE;
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
			output.emplace_back(std::move(Resample(frame)));
		}
		return output;
	}
}