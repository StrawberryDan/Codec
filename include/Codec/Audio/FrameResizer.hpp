#pragma once



//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Frame.hpp"
#include <cstddef>
#include <queue>
#include "Strawberry/Core/Option.hpp"


namespace Strawberry::Codec::Audio
{
	/// Class for rearranging the samples in audio frames such that all
	/// audio frames have a given number of samples.
	class FrameResizer
	{
	public:
		/// Constructor
		/// @param outputFrameSize The number of samples output audio frames should have
		FrameResizer(const FrameFormat& format, size_t outputFrameSize);

		FrameResizer(const FrameResizer& rhs) = delete;
		FrameResizer& operator=(const FrameResizer& rhs) = delete;
		FrameResizer(FrameResizer&& rhs) = delete;
		FrameResizer& operator=(FrameResizer&& rhs) = delete;
		/// Processes an input frame and outputs some number
		/// output audio frames with the size given in the constructor.
		/// @param frame The input frame
		void SendFrame(Frame frame);

		Core::Option<Frame> ReadFrame();

		bool IsOutputAvailable() const;

		const FrameFormat& GetFormat() const { return mFrameFormat; }


	private:
		const size_t        mOutputFrameSize;
		const FrameFormat   mFrameFormat;
		Core::Option<Frame> mWorkingFrame;
		std::queue<Frame>   mInputFrames;
	};
}
