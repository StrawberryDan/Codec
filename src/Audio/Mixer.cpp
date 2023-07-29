//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include <libavutil/channel_layout.h>
/// Strawberry Codec
#include "Codec/Audio/Mixer.hpp"
// Lib Format
#include "fmt/format.h"


constexpr size_t FRAME_SIZE = 48000 / 32;
constexpr int    FORMAT = AV_SAMPLE_FMT_DBL;

;
namespace Strawberry::Codec::Audio
{
	Mixer::Mixer(size_t trackCount)
		: mTrackCount(trackCount)
		, mResamplers()
	{
		FrameFormat format(48000, FORMAT, AV_CHANNEL_LAYOUT_STEREO);

		for (int i = 0; i < trackCount; i++)
		{
			mResamplers.emplace_back(format);
		}
	}


	void Mixer::Send(size_t trackIndex, const Frame& frame)
	{
		Core::Assert(trackIndex >= 0 && trackIndex < mTrackCount);
		mResamplers[trackIndex].SendFrame(frame);
	}


	Core::Option<Frame> Mixer::ReceiveFrame()
	{
		FrameFormat format(48000, FORMAT, AV_CHANNEL_LAYOUT_STEREO);

		std::vector<Frame> inputs;
		inputs.reserve(mTrackCount);
		int tracksReadFrom = 0;
		for (int i = 0; i < mTrackCount; i++)
		{
			auto frame = mResamplers[i].ReadFrame();
			if (frame) tracksReadFrom += 1;
			inputs.emplace_back(std::move(frame.UnwrapOr(Frame::Silence(format, FRAME_SIZE))));
		}

		if (tracksReadFrom == 0) return {};

		Frame frame = Frame::Silence(format, FRAME_SIZE);
		for (int inputIndex = 0; inputIndex < inputs.size(); inputIndex++)
		{
			for (int sampleIndex = 0; sampleIndex < FRAME_SIZE * format.channels.nb_channels; sampleIndex++)
			{
				for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
				{
					if (frame->data[i])
					{
						auto& input = reinterpret_cast<double*>(inputs[inputIndex]->data[i])[sampleIndex];
						auto& output = reinterpret_cast<double*>(frame->data[i])[sampleIndex];
						output += input / tracksReadFrom;
					}
				}
			}
		}

		return frame;
	}
}