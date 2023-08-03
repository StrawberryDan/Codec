//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include <libavutil/channel_layout.h>
/// Strawberry Codec
#include "Codec/Audio/Mixer.hpp"
// Lib Format
#include "fmt/format.h"



namespace Strawberry::Codec::Audio
{
	Mixer::Mixer(const FrameFormat& outputFormat, size_t outputFrameSize)
			: mOutputFormat(outputFormat)
			, mOutputFrameSize(outputFrameSize)
	{

	}


	Frame Mixer::ReadFrame()
	{
		// Erase dead channels
		std::erase_if(mInputChannels, [](std::weak_ptr<InputChannel>& x) { return x.expired(); });

		// Mix Input Channels
		Frame result = Frame::Silence(mOutputFormat, mOutputFrameSize);
		for (auto& channel : mInputChannels)
		{
			auto frame = channel.lock()->ReadFrame();
			result.Mix(frame);
		}

		return result;
	}


	bool Mixer::IsEmpty() const
	{
		return std::all_of(
			mInputChannels.begin(), mInputChannels.end(),
			[](auto& x) { return !x.lock()->IsOutputAvailable(); });
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
		, mFrameResizer(outputFormat, outputFrameSize)
	{}


	bool Mixer::InputChannel::IsOutputAvailable() const
	{
		bool a = !mFrameBuffer.empty();
		bool b = mFrameResizer.IsOutputAvailable();
		bool c = mResampler.IsOutputAvailable();
		return a || b || c;
	}


	void Mixer::InputChannel::EnqueueFrame(Frame frame)
	{
		Core::Assert(frame->sample_rate > 0);
		mFrameBuffer.emplace_back(std::move(frame));
	}


	Frame Mixer::InputChannel::ReadFrame()
	{
		while (true)
		{
			Core::Option<Frame> result = mFrameResizer.ReadFrame();
			if (result) return result.Unwrap();

			result = mResampler.ReadFrame();
			if (result)
			{
				mFrameResizer.SendFrame(result.Unwrap());
				continue;
			}

			result = mFrameBuffer.empty() ? Core::NullOpt : Core::Option(std::move(mFrameBuffer.front()));
			if (result)
			{
				Core::Assert(result.Value()->sample_rate > 0);
				mResampler.SendFrame(result.Unwrap());
				mFrameBuffer.pop_front();
				continue;
			}

			return Frame::Silence(mOutputFormat, mOutputFrameSize);
		}
	}
}