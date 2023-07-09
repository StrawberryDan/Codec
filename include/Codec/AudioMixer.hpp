#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
/// Codec
#include "AudioFrameResizer.hpp"
#include "Codec/Resampler.hpp"
#include "FilterGraph.hpp"
#include "Frame.hpp"
/// Strawberry Libraries
#include "Strawberry/Core/Option.hpp"


namespace Strawberry::Codec
{
	class AudioMixer
	{
	public:
		AudioMixer(size_t trackCount);


		void Send(size_t trackIndex, const Frame& frame);
		Core::Option<Frame> ReceiveFrame();

	private:
		size_t mTrackCount;
		std::vector<Resampler> mResamplers;
	};
}
