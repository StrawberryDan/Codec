#include <fmt/format.h>
#include "Codec/Audio/FrameResizer.hpp"



#include "Strawberry/Core/Assert.hpp"



namespace Strawberry::Codec::Audio
{
	FrameResizer::FrameResizer(const FrameFormat& format, size_t outputFrameSize)
		: mOutputFrameSize(outputFrameSize)
		, mFrameFormat(format)
		, mWorkingFrame(Frame::Silence(format, 1))
	{}



	void FrameResizer::SendFrame(Frame frame)
	{
		Core::Assert(frame.GetFormat() == mFrameFormat);
		mInputFrames.emplace(std::move(frame));
	}


	Core::Option<Frame> FrameResizer::ReadFrame()
	{
		while (true)
		{
			if (mWorkingFrame.GetNumSamples() == mOutputFrameSize)
			{
				auto result = std::move(mWorkingFrame);
				mWorkingFrame = Frame::Silence(mFrameFormat, 0);
				return result;
			}

			if (mWorkingFrame.GetNumSamples() > mOutputFrameSize)
			{
				auto [result, remainder] = mWorkingFrame.Split(mOutputFrameSize);
				mWorkingFrame = std::move(remainder);
				return result;
			}

			if (!mInputFrames.empty())
			{
				mWorkingFrame.Append(std::move(mInputFrames.front()));
				mInputFrames.pop();
				continue;
			}

			return Core::NullOpt;
		}
	}


	bool FrameResizer::IsOutputAvailable() const
	{
		return mWorkingFrame.GetNumSamples() > 0 && !mInputFrames.empty();
	}
}