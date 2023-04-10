#pragma once



#include <cstdint>
#include <vector>
#include <optional>
#include "Packet.hpp"
#include "Frame.hpp"



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
		static int CalculateBitrate(AVCodecContext* context);



	private:
		AVCodecContext* mContext;
		AVCodecParameters *mParameters;
		int64_t mPTS;
	};
}
