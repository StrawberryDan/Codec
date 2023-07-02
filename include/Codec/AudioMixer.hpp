#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
/// Codec
#include "FilterGraph.hpp"
#include "Frame.hpp"
/// Strawberry Libraries
#include "Strawberry/Core/Option.hpp"



namespace Strawberry::Codec
{
	class AudioMixer
			: private FilterGraph
	{
	public:
		AudioMixer();


		void SendFrame(unsigned int channel, Frame frame);
		Core::Option<Frame> RecvFrame();


		bool OutputAvailable() { return FilterGraph::OutputAvailable(0); }


	private:
		void SetUpMixer(unsigned int inputCount);
		unsigned int mMixerSize = 2;
	};
}
