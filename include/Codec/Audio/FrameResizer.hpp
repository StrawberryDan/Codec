#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Frame.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include <cstddef>
#include <queue>


namespace Strawberry::Codec::Audio
{
	/// Class for rearranging the samples in audio frames such that all
	/// audio frames have a given number of samples.
	class FrameResizer
	{
	public:
		enum class Mode
		{
			YieldAvailable,
			WaitForFullFrames,
		};


	public:
		/// Constructor
		/// @param outputFrameSize The number of samples output audio frames should have
		explicit FrameResizer(size_t outputFrameSize);

		FrameResizer(const FrameResizer& rhs)                = delete;
		FrameResizer& operator=(const FrameResizer& rhs)     = delete;
		FrameResizer(FrameResizer&& rhs) noexcept            = default;
		FrameResizer& operator=(FrameResizer&& rhs) noexcept = default;
		/// Processes an input frame and outputs some number
		/// output audio frames with the size given in the constructor.
		/// @param frame The input frame
		void          SendFrame(Frame frame);

		Core::Optional<Frame> ReadFrame(Mode mode);

		[[nodiscard]] bool IsOutputAvailable(Mode mode) const;


	private:
		size_t              mOutputFrameSize;
		Core::Optional<Frame> mWorkingFrame;
		std::queue<Frame>   mInputFrames;
	};
} // namespace Strawberry::Codec::Audio
