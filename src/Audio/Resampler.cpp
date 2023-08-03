#include <fmt/format.h>
#include <Strawberry/Core/Logging.hpp>
#include "Codec/Audio/Resampler.hpp"
#include "Strawberry/Core/Utilities.hpp"


#include "Strawberry/Core/Assert.hpp"


extern "C"
{
#include "libavutil/opt.h"
}



using Strawberry::Core::Assert;
using Strawberry::Core::Take;


namespace Strawberry::Codec::Audio
{
	Resampler::Resampler(FrameFormat outputFormat)
		: mOutputFormat(outputFormat)
		, mContext(swr_alloc())
	{
		Core::Assert(mContext != nullptr);
	}


	Resampler::~Resampler()
	{
		if (mContext) swr_free(&mContext);
		Core::Assert(mContext == nullptr);
	}


	void Resampler::SendFrame(Frame frame)
	{
		Core::Assert(frame->sample_rate > 0);
		mInputFrames.emplace(std::move(frame));
	}


	Core::Option<Frame> Resampler::ReadFrame()
	{
		if (!IsOutputAvailable()) return Core::NullOpt;


		Frame input(std::move(mInputFrames.front()));
		mInputFrames.pop();
		Core::Assert(input->sample_rate > 0);
		Frame output = Frame::Allocate();
		output->sample_rate = mOutputFormat.sampleRate;
		output->ch_layout   = mOutputFormat.channels;
		output->format      = mOutputFormat.sampleFormat;


		while (true)
		{
			auto result = swr_convert_frame(mContext, *output, *input);
			if (result == 0)
			{
				return output;
			}
			else if (result == AVERROR_INPUT_CHANGED)
			{
				swr_close(mContext);
				continue;
			}
			else
			{
				char buffer[2048]{'\0'};
				av_make_error_string(buffer, 2048, result);
				Core::Logging::Error("Resampler Error! {}", buffer);
				Core::Unreachable();
			}
		}
	}


	bool Resampler::IsOutputAvailable() const
	{
		return !mInputFrames.empty();
	}
}