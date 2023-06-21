#pragma once



#include "Frame.hpp"
#include <stddef.h>
#include <vector>
#include "Strawberry/Core/Option.hpp"



namespace Strawberry::Codec
{
	/// Class for rearranging the samples in audio frames such that all
	/// audio frames have a given number of samples.
	class AudioFrameResizer
	{
	public:
		/// Constructor
		/// @param sampleCount The number of samples output audio frames should have
		AudioFrameResizer(size_t sampleCount);
		/// Processes an input frame and outputs some number
		/// output audio frames with the size given in the constructor.
		/// @param frame The input frame
		std::vector<Frame> Process(Frame frame);
		/// Flush the frame buffer and return the result
		Core::Option<Frame> Flush();


	private:
		/// Returns the number of samples contained in all the frames
		/// in the frame buffer.
		size_t BufferedSampleCount() const;


	private:
		/// The target sample count for output frames
		const size_t		mTargetSampleCount;
		/// Buffer for frames which have not been output yet.
		std::vector<Frame>	mFrameBuffer;
	};
}
