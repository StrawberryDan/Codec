#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
/// Codec
#include "FrameResizer.hpp"
#include "Codec/Audio/Resampler.hpp"
#include "Frame.hpp"
/// Strawberry Libraries
#include "Strawberry/Core/Option.hpp"
/// Standard Library
#include <unordered_set>
#include <memory>
#include <list>


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
		std::list<std::weak_ptr<InputChannel>> mInputChannels;
	};


	class Mixer::InputChannel
	{
		friend class Mixer;


	public:
		InputChannel(const FrameFormat& outputFormat, size_t outputFrameSize);


		bool IsOutputAvailable() const;


		void EnqueueFrame(Frame frame);


	protected:
		Frame ReadFrame();


	private:
		const FrameFormat mOutputFormat;
		const size_t      mOutputFrameSize;
		std::deque<Frame>  mFrameBuffer;
		Resampler         mResampler;
		FrameResizer      mFrameResizer;
	};
}
