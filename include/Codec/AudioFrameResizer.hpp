#pragma once



//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Frame.hpp"
#include <stddef.h>
#include <vector>
#include "Strawberry/Core/Option.hpp"
#include "FilterGraph.hpp"


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

		AudioFrameResizer(const AudioFrameResizer& rhs) = delete;
		AudioFrameResizer& operator=(const AudioFrameResizer& rhs) = delete;
		AudioFrameResizer(AudioFrameResizer&& rhs) = default;
		AudioFrameResizer& operator=(AudioFrameResizer&& rhs) = default;
		/// Processes an input frame and outputs some number
		/// output audio frames with the size given in the constructor.
		/// @param frame The input frame
		void SendFrame(Frame frame);

		Core::Option<Frame> ReadFrame();



	private:
		void SetupFilterGraph(const AudioFrameFormat& format);


	private:
		/// The target sample count for output frames
		const size_t		mTargetSampleCount;
		/// Filter Graph to run frames through
		Core::Option<FilterGraph> mFilterGraph;

		InputFilter* mFilterGraphInput = nullptr;

		BufferSink* mFilterGraphOutput = nullptr;


		Core::Option<AudioFrameFormat> mLastFrameFormat;
	};
}
