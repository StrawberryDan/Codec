#pragma once



#include "AudioFrameResizer.hpp"
#include "Frame.hpp"
#include "Packet.hpp"
#include "Resampler.hpp"
#include "Strawberry/Core/Option.hpp"
#include <cstdint>
#include <vector>



namespace Strawberry::Codec
{
	class Encoder
	{
	public:
		Encoder(AVCodecID codecID, AVChannelLayout channelLayout);
		~Encoder();

		std::vector<Packet>  Encode(const Frame& frame);
		Core::Option<Packet> Flush();

		inline       AVCodecContext* operator*()        { return mContext; }
		inline const AVCodecContext* operator*()  const { return mContext; }
		inline       AVCodecContext* operator->()       { return mContext; }
		inline const AVCodecContext* operator->() const { return mContext; }

		[[nodiscard]] AVCodecParameters* Parameters() const;



	private:
		AVCodecContext*					mContext;
		AVCodecParameters*				mParameters;
		Core::Option<Resampler>			mFrameResampler;
		Core::Option<AudioFrameResizer>	mFrameResizer;
	};
}
