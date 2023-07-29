#pragma once



#include <vector>
#include "Frame.hpp"
#include "AudioFrameFormat.hpp"
#include "Strawberry/Core/Option.hpp"


extern "C"
{
#include "libswresample/swresample.h"
#include "libavutil/channel_layout.h"
}



namespace Strawberry::Codec::Audio
{
	class Resampler
	{
	public:
		explicit Resampler(FrameFormat targetFormat);


		void SendFrame(Frame frame);
		Core::Option<Frame> ReadFrame();


	private:

	};
}