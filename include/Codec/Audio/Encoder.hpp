#pragma once


#include "Codec/Audio/Frame.hpp"
#include "Codec/Audio/FrameResizer.hpp"
#include "Codec/Audio/Resampler.hpp"
#include "Codec/Packet.hpp"
#include "Strawberry/Core/Util/Optional.hpp"
#include <cstdint>
#include <vector>

namespace Strawberry::Codec::Audio
{
	class Encoder
	{
	public:
		Encoder(AVCodecID codecID, AVChannelLayout channelLayout);
		Encoder(Encoder&& rhs);
		~Encoder();


		void                Send(Frame frame);
		std::vector<Packet> Receive();
		std::vector<Packet> Flush();

		inline AVCodecContext* operator*() { return mContext; }

		inline const AVCodecContext* operator*() const { return mContext; }

		inline AVCodecContext* operator->() { return mContext; }

		inline const AVCodecContext* operator->() const { return mContext; }

		[[nodiscard]] AVCodecParameters* Parameters() const;


	private:
		AVCodecContext*              mContext;
		AVCodecParameters*           mParameters;
		Core::Optional<Resampler>    mFrameResampler;
		Core::Optional<FrameResizer> mFrameResizer;
		std::deque<Frame>            mFrameBuffer;
	};
} // namespace Strawberry::Codec::Audio
