#include <fmt/format.h>
#include "Codec/Resampler.hpp"
#include "Strawberry/Core/Utilities.hpp"


#include "Strawberry/Core/Assert.hpp"


extern "C"
{
#include "libavfilter/buffersink.h"
}



using Strawberry::Core::Assert;
using Strawberry::Core::Take;


namespace Strawberry::Codec
{
	Resampler::Resampler(AudioFrameFormat targetFormat)
			: mTargetFormat(targetFormat) {}


	void Resampler::SendFrame(Frame frame)
	{
		if (mLastFrameFormat != AudioFrameFormat(frame))
		{
			SetupGraph(AudioFrameFormat(frame));
			mLastFrameFormat = AudioFrameFormat(frame);
		}

		mInput->SendFrame(std::move(frame));
	}


	Core::Option<Frame> Resampler::ReadFrame()
	{
		auto output = mOutput->ReadFrame();
		if (mOutputFrameSize && output)
		{
			Core::Assert(*mOutputFrameSize == (*output)->nb_samples);
		}

		return output;
	}


	void Resampler::SetOutputFrameSize(unsigned int frameSize)
	{
		mOutputFrameSize = frameSize;
	}


	void Resampler::SetupGraph(AudioFrameFormat format)
	{
		mFilterGraph.Emplace(MediaType::Audio);

		char inChannelString[1024]{0};
		auto res = av_channel_layout_describe(&format.channels, inChannelString, sizeof(inChannelString));
		Core::Assert(res >= 0);

		char outChannelString[1024]{0};
		res = av_channel_layout_describe(&mTargetFormat.channels, outChannelString, sizeof(outChannelString));
		Core::Assert(res >= 0);

		auto args = fmt::format("isr={}:osr={}:isf={}:osf={}:ichl={}:ochl={}",
								format.sampleRate, mTargetFormat.sampleRate,
								format.sampleFormat, mTargetFormat.sampleFormat,
								inChannelString, outChannelString);
		mInput = mFilterGraph->AddInputAudioBuffer(0, format).Unwrap();
		auto resampler = mFilterGraph->AddFilter("aresample", "resampler",
												 args).Unwrap();
		mOutput = mFilterGraph->AddOutput(0).Unwrap();

		mInput->Link(*resampler, 0, 0);
		resampler->Link(*mOutput, 0, 0);

		Core::Assert(mFilterGraph->Configure());

		if (mOutputFrameSize)
		{
			av_buffersink_set_frame_size(**mOutput, *mOutputFrameSize);
		}
	}
}