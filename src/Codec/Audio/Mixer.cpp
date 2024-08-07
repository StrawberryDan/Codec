//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include <utility>

#include "Codec/Audio/Mixer.hpp"
// Lib Format
#include "fmt/format.h"
// Standard Library
#include <algorithm>


namespace Strawberry::Codec::Audio
{
	Mixer::Mixer(FrameFormat outputFormat, size_t outputFrameSize)
		: mOutputFormat(std::move(outputFormat))
		, mOutputFrameSize(outputFrameSize) {}


	Frame Mixer::ReadFrame()
	{
		// Erase dead channels
		std::erase_if(mInputChannels,
		              [](std::shared_ptr<InputChannel>& x)
		              {
			              return x.unique() && !x->IsOutputAvailable();
		              });

		// Mix Input Channels
		Frame result = Frame::Silence(mOutputFormat, mOutputFrameSize);
		for (auto& channel: mInputChannels)
		{
			auto frame = channel->ReadFrame();
			result     = result.Mix(frame);
		}

		return result;
	}


	bool Mixer::IsEmpty() const
	{
		return std::all_of(mInputChannels.begin(),
		                   mInputChannels.end(),
		                   [](auto& x)
		                   {
			                   return !x->IsOutputAvailable();
		                   });
	}


	std::shared_ptr<Mixer::InputChannel> Mixer::CreateInputChannel()
	{
		auto channel = std::make_shared<InputChannel>(mOutputFormat, mOutputFrameSize);
		mInputChannels.emplace_back(channel);
		return channel;
	}


	Mixer::InputChannel::InputChannel(const FrameFormat& outputFormat, size_t outputFrameSize)
		: mOutputFormat(outputFormat)
		, mOutputFrameSize(outputFrameSize)
		, mResampler(outputFormat)
		, mFrameResizer(outputFrameSize) {}


	bool Mixer::InputChannel::IsOutputAvailable() const
	{
		bool a = !mFrameBuffer.Lock()->empty();
		bool b = mFrameResizer.IsOutputAvailable(FrameResizer::Mode::WaitForFullFrames);
		bool c = mResampler.IsOutputAvailable();
		return a || b || c;
	}


	size_t Mixer::InputChannel::QueueLength() const
	{
		size_t sum         = 0;
		auto   frameBuffer = mFrameBuffer.Lock();
		for (const auto& frame: *frameBuffer)
		{
			sum += frame->nb_samples;
		}
		return sum;
	}


	void Mixer::InputChannel::EnqueueFrame(Frame frame)
	{
		Core::Assert(frame->sample_rate > 0);
		mFrameBuffer.Lock()->emplace_back(std::move(frame));
	}


	Frame Mixer::InputChannel::ReadFrame()
	{
		while (true)
		{
			Core::Optional<Frame> result = mFrameResizer.ReadFrame(FrameResizer::Mode::WaitForFullFrames);
			if (result) return result.Unwrap();

			result = mResampler.ReadFrame();
			if (result)
			{
				mFrameResizer.SendFrame(result.Unwrap());
				continue;
			}

			auto frameBuffer = mFrameBuffer.Lock();
			result           = frameBuffer->empty() ? Core::NullOpt : Core::Optional(std::move(frameBuffer->front()));
			if (result)
			{
				Core::Assert(result.Value()->sample_rate > 0);
				mResampler.SendFrame(result.Unwrap());
				frameBuffer->pop_front();
				continue;
			}

			return Frame::Silence(mOutputFormat, mOutputFrameSize);
		}
	}
} // namespace Strawberry::Codec::Audio
