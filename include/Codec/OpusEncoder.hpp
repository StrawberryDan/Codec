#pragma once



#include "AudioFrameResizer.hpp"
#include "Core/Option.hpp"
#include "Frame.hpp"
#include "Packet.hpp"
#include "Resampler.hpp"
#include <cstdint>
#include <vector>



namespace Strawberry::Codec
{
	class OpusEncoder
	{
	public:
		OpusEncoder();
		~OpusEncoder();

		std::vector<Packet> Encode(const Frame& frame);

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
