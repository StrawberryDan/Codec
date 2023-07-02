//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/AudioMixer.hpp"


namespace Strawberry::Codec
{
	AudioMixer::AudioMixer(unsigned int channelCount)
		: FilterGraph(MediaType::Audio)
	{
		AddOutput("");
		for (int i = 0; i < channelCount; i++)
		{
			AddInput("");
		}

		Configure();
		Start();
	}


	void AudioMixer::SendFrame(unsigned int channel, Frame frame)
	{
		FilterGraph::SendFrame(channel, frame);
	}


	Core::Option<Frame> AudioMixer::RecvFrame()
	{
		return FilterGraph::RecvFrame(0);
	}
}