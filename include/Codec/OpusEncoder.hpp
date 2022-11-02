#pragma once



#include <cstdint>
#include <vector>
#include <optional>
#include "Sample.hpp"
#include "Packet.hpp"



extern "C"
{
#include "libavcodec/avcodec.h"
}



namespace Strawberry::Codec
{
	class OpusEncoder
	{
	public:
	    OpusEncoder();
	    ~OpusEncoder();

	    std::vector<Packet> Encode(const Samples& samples);
	    std::vector<Packet> Finish();

	    inline       AVCodecContext* operator*()        { return mContext; }
	    inline const AVCodecContext* operator*()  const { return mContext; }
	    inline       AVCodecContext* operator->()       { return mContext; }
	    inline const AVCodecContext* operator->() const { return mContext; }

	    [[nodiscard]] AVCodecParameters* Parameters() const;

	private:
	    AVCodecContext* mContext;
	    AVCodecParameters *mParameters;
	    int64_t mPTS;
	    Samples mSampleBuffer;
	};
}
