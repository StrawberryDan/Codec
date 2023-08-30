//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include <utility>

#include "Codec/Audio/Resampler.hpp"
// Core
#include "Strawberry/Core/Util/Assert.hpp"
#include "Strawberry/Core/Util/Logging.hpp"
#include "Strawberry/Core/Util/Utilities.hpp"


extern "C" {
#include "libavutil/opt.h"
}


using Strawberry::Core::Assert;
using Strawberry::Core::Take;


namespace Strawberry::Codec::Audio
{
	Resampler::Resampler(FrameFormat outputFormat)
		: mOutputFormat(std::move(outputFormat))
		, mContext(swr_alloc())
	{
		Core::Assert(mContext != nullptr);
		Core::Assert(mOutputFormat.GetSampleRate() > 0);
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

	Core::Optional<Frame> Resampler::ReadFrame()
	{
		if (!IsOutputAvailable()) return Core::NullOpt;

		// Read our next buffered frame. Early return if it's already in the right format.
		Core::Assert(!mInputFrames.empty());
		Frame input(std::move(mInputFrames.front()));
		mInputFrames.pop();
		Core::Assert(input->sample_rate > 0);
		if (input.GetFormat() == mOutputFormat) { return input; }


		Frame output        = Frame::Allocate();
		output->sample_rate = mOutputFormat.GetSampleRate();
		auto result         = av_channel_layout_copy(&output->ch_layout, mOutputFormat.GetChannels());
		Core::Assert(result == 0);
		output->format = mOutputFormat.GetSampleFormat();


		while (true)
		{
			auto result = swr_convert_frame(mContext, *output, *input);
			if (result == 0) { return output; }
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
} // namespace Strawberry::Codec::Audio