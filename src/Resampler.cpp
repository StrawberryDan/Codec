#include "Codec/Resampler.hpp"
#include "Strawberry/Core/Utilities.hpp"


#include "Strawberry/Core/Assert.hpp"



using Strawberry::Core::Assert;
using Strawberry::Core::Take;



namespace Strawberry::Codec
{
	Resampler::Resampler(unsigned int targetSampleRate, AVChannelLayout targetLayout, AVSampleFormat targetFormat, const AVCodecParameters* codecParameters)
		: mTargetSampleRate(targetSampleRate)
		, mTargetChannelLayout(targetLayout)
		, mTargetSampleFormat(targetFormat)
		, mCodecParameters(codecParameters)
		, mSwrContext(nullptr)
	{
		auto source_channel_layout = codecParameters->ch_layout;
		auto result = swr_alloc_set_opts2(
				&mSwrContext,
				&targetLayout,
				targetFormat,
				static_cast<int>(targetSampleRate),
				&codecParameters->ch_layout,
				static_cast<AVSampleFormat>(codecParameters->format),
				codecParameters->sample_rate,
				0,
				nullptr);
		Assert(result == 0);
	}



	Resampler::Resampler(Resampler&& other) noexcept
		: mTargetSampleRate(other.mTargetSampleRate)
		, mTargetChannelLayout(other.mTargetChannelLayout)
		, mTargetSampleFormat(other.mTargetSampleFormat)
		, mCodecParameters(other.mCodecParameters)
		, mSwrContext(Take(other.mSwrContext))
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
		output->ch_layout	= mTargetChannelLayout;
		output->format		= mTargetSampleFormat;
		output->sample_rate	= static_cast<int>(mTargetSampleRate);
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