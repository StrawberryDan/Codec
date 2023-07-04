//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/PTSSetter.hpp"



namespace Strawberry::Codec
{
	PTSSetter::PTSSetter()
	{}


	void PTSSetter::SendFrame(Frame frame)
	{
		if (mLastFrameFormat != AudioFrameFormat(frame))
		{
			SetupGraph(AudioFrameFormat(frame));
			mLastFrameFormat.Emplace(frame);
		}

		mInput->SendFrame(frame);
	}


	Core::Option<Frame> PTSSetter::ReadFrame()
	{
		return mOutput->ReadFrame();
	}


	void PTSSetter::SetupGraph(AudioFrameFormat format)
	{
		mFilterGraph.Emplace(MediaType::Audio);

		mInput = mFilterGraph->AddInputAudioBuffer(0, format).Unwrap();
		auto filter = mFilterGraph->AddFilter("asetpts", "pts", "expr=N/SR/TB").Unwrap();
		mOutput = mFilterGraph->AddOutput(0).Unwrap();

		mInput->Link(*filter, 0, 0);
		filter->Link(*mOutput, 0, 0);
		auto result = mFilterGraph->Configure();
		Core::Assert(result);
	}
}