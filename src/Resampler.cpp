#include "Codec/Resampler.hpp"
#include "Strawberry/Core/Utilities.hpp"


#include "Strawberry/Core/Assert.hpp"



using Strawberry::Core::Assert;
using Strawberry::Core::Take;



namespace Strawberry::Codec
{
	Resampler::Resampler(unsigned int targetSampleRate, AVChannelLayout targetLayout, AVSampleFormat targetFormat)
		: mTargetSampleRate(targetSampleRate)
		, mTargetChannelLayout(targetLayout)
		, mTargetSampleFormat(targetFormat)
		, mSwrContext(nullptr)
	{}



	Resampler::Resampler(Resampler&& other) noexcept
		: mTargetSampleRate(other.mTargetSampleRate)
		, mTargetChannelLayout(other.mTargetChannelLayout)
		, mTargetSampleFormat(other.mTargetSampleFormat)
		, mSwrContext(Take(other.mSwrContext))
	{}



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
		if (mSwrContext) swr_free(&mSwrContext);
	}



	Frame Resampler::Resample(const Frame& input)
	{
		auto result = swr_alloc_set_opts2(
				&mSwrContext,
				&mTargetChannelLayout,
				mTargetSampleFormat,
				static_cast<int>(mTargetSampleRate),
				&input->ch_layout,
				static_cast<AVSampleFormat>(input->format),
				input->sample_rate,
				0,
				nullptr);
		Assert(result == 0);

		Assert(mSwrContext != nullptr);

		Frame output;
		output->ch_layout	= mTargetChannelLayout;
		output->format		= mTargetSampleFormat;
		output->sample_rate	= static_cast<int>(mTargetSampleRate);
		result = swr_convert_frame(mSwrContext, *output, *input);
		Assert(result == 0);
		Assert(output->ch_layout.nb_channels == mTargetChannelLayout.nb_channels);
		Assert(output->ch_layout.order == mTargetChannelLayout.order);
		Assert(output->format == mTargetSampleFormat);
		Assert(output->sample_rate == mTargetSampleRate);


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