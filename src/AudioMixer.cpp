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
	AudioMixer::AudioMixer()
		: FilterGraph(MediaType::Audio)
	{
		auto output = AddOutput(0, "").Unwrap();
		SetUpMixer(0);
	}


	void AudioMixer::SendFrame(unsigned int channel, Frame frame)
	{
		if (channel >= mMixerSize)
		{
			SetUpMixer(channel + 1);
		}


		BufferSource* source = GetInput(channel);
		if (GetInput(channel) == nullptr || source->GetSampleRate() != frame->sample_rate || source->GetSampleFormat() != frame->format || source->GetChannelCount() != frame->ch_layout.nb_channels || source->GetChannelLayout() != frame->channel_layout)
		{
			if (GetInput(channel))
				RemoveInput(channel);
			Filter* newInput = AddAudioInput(channel, frame->sample_rate, frame->format, frame->ch_layout.nb_channels, frame->channel_layout).Unwrap();
			// RemoveFilter(fmt::format("formatter-{}", channel));
			// auto formatter = AddFilter("aformat", fmt::format("formatter-{}", GetInputCount() - 1), "channel_layouts=stereo").Unwrap();
			auto mixer = GetFilter("mixer").Unwrap();
			newInput->Link(*mixer, 0, channel);
			// newInput->Link(*formatter, 0, 0);
			// formatter->Link(*mixer, 0, channel);
		}

		FilterGraph::SendFrame(channel, std::move(frame));
	}


	Core::Option<Frame> AudioMixer::RecvFrame()
	{
#if !NDEBUG
		Configure();
#endif // !NDEBUG

		auto result = FilterGraph::RecvFrame(0);
		return result;
	}


	void AudioMixer::SetUpMixer(unsigned int inputCount)
	{
		inputCount = std::max<unsigned int>(2, inputCount);
		RemoveFilter("mixer");
		auto mixer = AddFilter("amix", "mixer", fmt::format("inputs={}", inputCount)).Unwrap();
		mixer->Link(*GetOutput(0), 0, 0);

		for (auto& [i, input] : GetInputPairs())
		{
			input->Link(*mixer, 0, i);
		}

		mMixerSize = inputCount;
	}
}