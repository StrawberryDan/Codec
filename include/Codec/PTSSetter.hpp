#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------



#include "FilterGraph.hpp"


namespace Strawberry::Codec
{
	class PTSSetter
	{
	public:
		PTSSetter();


		void SendFrame(Frame frame);
		Core::Option<Frame> ReadFrame();


	private:
		void SetupGraph(AudioFrameFormat format);


	private:
		Core::Option<AudioFrameFormat> mLastFrameFormat;
		Core::Option<FilterGraph> mFilterGraph;
		InputFilter* mInput;
		BufferSink*  mOutput;
	};
}