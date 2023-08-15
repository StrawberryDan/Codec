#include "Codec/Audio/FrameResizer.hpp"
#include <fmt/format.h>


#include "Strawberry/Core/Util/Assert.hpp"


namespace Strawberry::Codec::Audio
{
	FrameResizer::FrameResizer(size_t outputFrameSize)
		: mOutputFrameSize(outputFrameSize)
		, mWorkingFrame()
	{
	}


	void FrameResizer::SendFrame(Frame frame)
	{
		Core::Assert(*frame);
		mInputFrames.emplace(std::move(frame));
	}


	Core::Option<Frame> FrameResizer::ReadFrame(FrameResizer::Mode mode)
	{
		while (true)
		{
			// Check if we are able to populate the working frame if we need to.
			if (!mWorkingFrame)
			{
				if (mInputFrames.empty()) return Core::NullOpt;
				mWorkingFrame = std::move(mInputFrames.front());
				mInputFrames.pop();
			}


			Core::Assert(mWorkingFrame.HasValue());

			// If the working frame is larger than our desired output then we split it.
			if (mWorkingFrame->GetNumSamples() > mOutputFrameSize)
			{
				auto [result, remainder] = mWorkingFrame->Split(mOutputFrameSize);
				mWorkingFrame            = std::move(remainder);

				Core::Assert(result->sample_rate > 0);
				return result;
			}
			// If the working frame is the right size we return it.
			else if (mWorkingFrame->GetNumSamples() == mOutputFrameSize)
			{
				return mWorkingFrame.Unwrap();
			}
			// If the mode is Yield Available, and we have not more input frames, then we yield what we have.
			else if (mode == Mode::YieldAvailable && mWorkingFrame->GetNumSamples() <= mOutputFrameSize && mInputFrames.empty())
			{
				return mWorkingFrame.Unwrap();
			}
			// Otherwise, we append frames to the working frame until it's big enough, or we run out of frames.
			else if (mWorkingFrame->GetNumSamples() < mOutputFrameSize && !mInputFrames.empty())
			{
				while (mWorkingFrame->GetNumSamples() < mOutputFrameSize && !mInputFrames.empty())
				{
					mWorkingFrame->Append(mInputFrames.front());
					mInputFrames.pop();
				}
				continue;
			}

			return Core::NullOpt;
		}
	}


	bool FrameResizer::IsOutputAvailable(Mode mode) const
	{
		switch (mode)
		{
			case Mode::WaitForFullFrames:
				return (mWorkingFrame.HasValue() && mWorkingFrame->GetNumSamples() > mOutputFrameSize) ||
					   !mInputFrames.empty();
			case Mode::YieldAvailable:
				return (mWorkingFrame.HasValue() && mWorkingFrame->GetNumSamples() > 0) || !mInputFrames.empty();
		}
	}
}// namespace Strawberry::Codec::Audio