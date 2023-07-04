#include <fmt/format.h>
#include "Codec/AudioFrameResizer.hpp"



#include "Strawberry/Core/Assert.hpp"



namespace Strawberry::Codec
{
	AudioFrameResizer::AudioFrameResizer(size_t sampleCount)
		: mTargetSampleCount(sampleCount)
	{}



	void AudioFrameResizer::SendFrame(Frame frame)
	{
		if (mLastFrameFormat != AudioFrameFormat(frame))
		{
			SetupFilterGraph(AudioFrameFormat(frame));
			mLastFrameFormat.Emplace(frame);
		}

		mFilterGraphInput->SendFrame(std::move(frame));
	}


	Core::Option<Frame> AudioFrameResizer::ReadFrame()
	{
		auto nextFrame = mFilterGraphOutput->PeekFrame();
		if (nextFrame && (*nextFrame)->nb_samples >= mTargetSampleCount)
		{
			Core::Assert((*nextFrame)->nb_samples == mTargetSampleCount);
			mFilterGraphOutput->ReadFrame().Unwrap();
			Core::Assert(*mLastFrameFormat == AudioFrameFormat(*nextFrame));
			return nextFrame;
		}

		return {};
	}


	void AudioFrameResizer::SetupFilterGraph(const AudioFrameFormat& format)
	{
		mFilterGraph.Emplace(MediaType::Audio);

		mFilterGraphInput = mFilterGraph->AddInputAudioBuffer(0, format).Unwrap();
		auto* resizer = mFilterGraph->AddFilter("asetnsamples", "resizer", fmt::format("n={}:p=0", mTargetSampleCount)).Unwrap();
		auto* pts = mFilterGraph->AddFilter("asetpts", "pts", "expr=NB_CONSUMED_SAMPLES").Unwrap();
		mFilterGraphOutput = mFilterGraph->AddOutput(0).Unwrap();

		mFilterGraphInput->Link(*resizer, 0, 0);
		resizer->Link(*pts, 0, 0);
		pts->Link(*mFilterGraphOutput, 0, 0);

		auto result = mFilterGraph->Configure();
		Core::Assert(result);
	}
}