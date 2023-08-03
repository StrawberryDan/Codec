#include <fmt/format.h>
#include "Codec/Audio/FrameResizer.hpp"



#include "Strawberry/Core/Assert.hpp"



namespace Strawberry::Codec::Audio
{
	FrameResizer::FrameResizer(const FrameFormat& format, size_t outputFrameSize)
		: mOutputFrameSize(outputFrameSize)
		, mFrameFormat(format)
		, mWorkingFrame()
	{}



	void FrameResizer::SendFrame(Frame frame)
	{
		mInputFrames.emplace(std::move(frame));
	}


	Core::Option<Frame> FrameResizer::ReadFrame()
	{
		while (true)
		{
			if (!mWorkingFrame)
			{
				if (mInputFrames.empty()) return Core::NullOpt;

				mWorkingFrame = std::move(mInputFrames.front());
				mInputFrames.pop();
			}

			Core::Assert(mWorkingFrame.HasValue());
			if (mWorkingFrame->GetNumSamples() == mOutputFrameSize)
			{
				return std::move(mWorkingFrame);
			}

			if (mWorkingFrame->GetNumSamples() > mOutputFrameSize)
			{
				auto [result, remainder] = mWorkingFrame->Split(mOutputFrameSize);
				mWorkingFrame = std::move(remainder);

				Core::Assert(result->sample_rate > 0);
				return result;
			}

			if (mWorkingFrame->GetNumSamples() < mOutputFrameSize && !mInputFrames.empty())
			{
				mWorkingFrame->Append(mInputFrames.front());
				mInputFrames.pop();
				continue;
			}

			return Core::NullOpt;
		}
	}


	bool FrameResizer::IsOutputAvailable() const
	{
		return !mInputFrames.empty() || (mWorkingFrame.HasValue() && mWorkingFrame->GetNumSamples() > mOutputFrameSize);
	}
}