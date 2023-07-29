#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
/// Codec
#include "AudioFrameResizer.hpp"
#include "Resampler.hpp"
#include "Frame.hpp"
/// Strawberry Libraries
#include "Strawberry/Core/Option.hpp"


namespace Strawberry::Codec::Audio
{
	class Mixer
	{
	public:
		Mixer(size_t trackCount);


		void Send(size_t trackIndex, const Frame& frame);
		Core::Option<Frame> ReceiveFrame();

	private:
		size_t mTrackCount;
		std::vector<Resampler> mResamplers;
	};
}
