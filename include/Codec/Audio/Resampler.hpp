#pragma once



#include <queue>
#include "Frame.hpp"
#include "FrameFormat.hpp"
#include "Strawberry/Core/Util/Option.hpp"


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
		explicit Resampler(FrameFormat outputFormat);
		Resampler(const Resampler&)            = delete;
		Resampler& operator=(const Resampler&) = delete;
		Resampler(Resampler&&)                 = delete;
		Resampler& operator=(Resampler&&)      = delete;
		~Resampler();


		const FrameFormat OutputFormat() const { return mOutputFormat; }


		void SendFrame(Frame frame);
		Core::Option<Frame> ReadFrame();
		bool IsOutputAvailable() const;


	private:
		Core::Option<FrameFormat> mInputFormat;
		const FrameFormat         mOutputFormat;
		SwrContext*               mContext;
		std::queue<Frame>         mInputFrames;
	};
}