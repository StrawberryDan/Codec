#pragma once


#include "Frame.hpp"
#include "FrameFormat.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include <queue>


extern "C"
{
#include "libavutil/channel_layout.h"
#include "libswresample/swresample.h"
}


namespace Strawberry::Codec::Audio
{
	class Resampler
	{
	public:
		explicit Resampler(FrameFormat outputFormat);
		Resampler(const Resampler&)            = delete;
		Resampler& operator=(const Resampler&) = delete;
		Resampler(Resampler&&)                 = default;
		Resampler& operator=(Resampler&&)      = default;
		~Resampler();


		[[nodiscard]] FrameFormat OutputFormat() const
		{
			return mOutputFormat;
		}


		void                  SendFrame(Frame frame);
		Core::Optional<Frame> ReadFrame();
		[[nodiscard]] bool    IsOutputAvailable() const;

	private:
		FrameFormat       mOutputFormat;
		SwrContext*       mContext;
		std::queue<Frame> mInputFrames;
	};
} // namespace Strawberry::Codec::Audio
