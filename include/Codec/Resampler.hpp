#pragma once



#include <vector>
#include "Frame.hpp"
#include "Filter.hpp"
#include "FilterGraph.hpp"
#include "AudioFrameFormat.hpp"


extern "C"
{
#include "libswresample/swresample.h"
#include "libavutil/channel_layout.h"
}



namespace Strawberry::Codec
{
	class Resampler
	{
	public:
		explicit Resampler(AudioFrameFormat targetFormat);


		void SendFrame(Frame frame);
		Core::Option<Frame> ReadFrame();


		void SetOutputFrameSize(unsigned int frameSize);


	private:
		void SetupGraph(AudioFrameFormat format);


	private:
		AudioFrameFormat mTargetFormat;
		Core::Option<unsigned int> mOutputFrameSize;
		Core::Option<AudioFrameFormat> mLastFrameFormat;
		Core::Option<FilterGraph> mFilterGraph;
		InputFilter* mInput;
		BufferSink* mOutput;
	};
}