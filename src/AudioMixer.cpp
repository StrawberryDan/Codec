//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include <libavutil/channel_layout.h>
#include "Codec/AudioMixer.hpp"
// Lib Format
#include "fmt/format.h"


extern "C"
{
#include "libavutil/channel_layout.h"
}


namespace Strawberry::Codec
{
	AudioMixer::AudioMixer(unsigned int channelCount)
		: FilterGraph(MediaType::Audio)
	{
		AddFilter("amix", "mixer", fmt::format("inputs={}", channelCount)).Unwrap();
	}


	void AudioMixer::SendFrame(unsigned int channel, Frame frame)
	{
		Core::Assert(channel <= GetInputCount());
		if (channel == GetInputCount())
		{
			Stop();
			Filter* newInput = nullptr;
			if (frame->channel_layout != 0)
				newInput = AddInput(fmt::format("sample_fmt={}:sample_rate={}:channel_layout={}", frame->format, frame->sample_rate, frame->channel_layout)).Unwrap();
			else
				newInput = AddInput(fmt::format("sample_fmt={}:channels={}:sample_rate={}", frame->format, frame->ch_layout.nb_channels, frame->sample_rate)).Unwrap();
			auto formatter = AddFilter("aformat", fmt::format("formatter-{}", GetInputCount() - 1), "channel_layouts=stereo").Unwrap();
			auto mixer = GetFilter("mixer").Unwrap();
			newInput->Link(*formatter, 0, 0);
			formatter->Link(*mixer, 0, GetInputCount() - 1);
		}

		FilterGraph::SendFrame(channel, frame);
	}


	Core::Option<Frame> AudioMixer::RecvFrame()
	{
		if (GetOutputCount() == 0)
		{
			Stop();
			auto output = AddOutput("").Unwrap();
			auto mixer = GetFilter("mixer").Unwrap();
			mixer->Link(*output, 0, 0);
			Configure();
			Start();
		}

		auto result = FilterGraph::RecvFrame(0);
		return result;
	}
}