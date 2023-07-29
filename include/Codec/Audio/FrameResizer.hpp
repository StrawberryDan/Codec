#pragma once



//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Frame.hpp"
#include <stddef.h>
#include <vector>
#include "Strawberry/Core/Option.hpp"


namespace Strawberry::Codec::Audio
{
	/// Class for rearranging the samples in audio frames such that all
	/// audio frames have a given number of samples.
	class FrameResizer
	{
	public:
		/// Constructor
		/// @param sampleCount The number of samples output audio frames should have
		FrameResizer(size_t sampleCount);

		FrameResizer(const FrameResizer& rhs) = delete;
		FrameResizer& operator=(const FrameResizer& rhs) = delete;
		FrameResizer(FrameResizer&& rhs) = default;
		FrameResizer& operator=(FrameResizer&& rhs) = default;
		/// Processes an input frame and outputs some number
		/// output audio frames with the size given in the constructor.
		/// @param frame The input frame
		void SendFrame(Frame frame);

		Core::Option<Frame> ReadFrame();


	private:

	};
}
