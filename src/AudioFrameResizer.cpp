#include "Codec/AudioFrameResizer.hpp"



#include "Strawberry/Core/Assert.hpp"



namespace Strawberry::Codec
{
	AudioFrameResizer::AudioFrameResizer(size_t sampleCount)
		: mTargetSampleCount(sampleCount)
	{}



	std::vector<Frame> AudioFrameResizer::Process(Frame frame)
	{
		mFrameBuffer.push_back(std::move(frame));

		std::vector<Frame> resized;
		while (BufferedSampleCount() > mTargetSampleCount)
		{
			Frame rFrame;
			rFrame->format = mFrameBuffer[0]->format;
			rFrame->ch_layout = mFrameBuffer[0]->ch_layout;

			while (rFrame->nb_samples < mTargetSampleCount)
			{
				auto nextBufferedFrame = mFrameBuffer.begin();

				size_t remainingSamplesNeeded = mTargetSampleCount - rFrame->nb_samples;
				if (remainingSamplesNeeded >= (*nextBufferedFrame)->nb_samples)
				{
					// Merge this frame in
					rFrame.Append(*nextBufferedFrame);
					mFrameBuffer.erase(nextBufferedFrame);
				}
				else
				{
					// Split the next frame up
					auto [needed, spare] = nextBufferedFrame->Split(remainingSamplesNeeded);
					rFrame.Append(needed);
					(*nextBufferedFrame) = std::move(spare);
				}
			}

			resized.push_back(std::move(rFrame));
		}

		return std::move(resized);
	}



	Core::Option<Frame> AudioFrameResizer::Flush()
	{
		if (mFrameBuffer.empty()) return {};

		Frame frame;
		for (const auto& stashFrame : mFrameBuffer)
		{
			frame.Append(stashFrame);
		}
		mFrameBuffer.clear();
		return frame;
	}



	size_t AudioFrameResizer::BufferedSampleCount() const
	{
		size_t samples = 0;
		for (const auto& frame : mFrameBuffer)
		{
			samples += frame->nb_samples;
		}
		return samples;
	}
}