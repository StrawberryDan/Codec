#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "FrameResizer.hpp"
#include "Codec/Audio/Resampler.hpp"
#include "Frame.hpp"
// Strawberry Libraries
#include "Strawberry/Core/Util/Option.hpp"
#include "Strawberry/Core/Sync/Mutex.hpp"
// Standard Library
#include <unordered_set>
#include <memory>
#include <list>
#include <deque>


namespace Strawberry::Codec::Audio
{
	class Mixer
	{
	public:
		class InputChannel;


	public:
		Mixer(const FrameFormat& outputFormat, size_t outputFrameSize);


		Frame ReadFrame();


		bool IsEmpty() const;


		std::shared_ptr<InputChannel> CreateInputChannel();


	private:
		const FrameFormat                      mOutputFormat;
		const size_t                           mOutputFrameSize;
		std::list<std::shared_ptr<InputChannel>> mInputChannels;
	};


	class Mixer::InputChannel
	{
		friend class Mixer;


	public:
		InputChannel(const FrameFormat& outputFormat, size_t outputFrameSize);


		/// Returns whether there are any queued samples in this channel.
		bool IsOutputAvailable() const;
		/// Returns the number of samples currently buffered.
		size_t QueueLength() const;
		/// Enqueues a frame in this channel's buffer.
		void EnqueueFrame(Frame frame);


	protected:
		Frame ReadFrame();


	private:
		const FrameFormat              mOutputFormat;
		const size_t                   mOutputFrameSize;
		Core::Mutex<std::deque<Frame>> mFrameBuffer;
		Resampler                      mResampler;
		FrameResizer                   mFrameResizer;
	};
}
