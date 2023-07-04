//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/Filter.hpp"
/// Strawberry Libraries
#include "Strawberry/Core/Assert.hpp"
/// Standard Library
#include <memory>
#include <Strawberry/Core/Option.hpp>


extern "C"
{
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}


namespace Strawberry::Codec
{
	Filter::Filter()
			: mFilterContext(nullptr)
	{}


	Filter::Filter(Filter&& rhs)
			: mFilterContext(std::exchange(rhs.mFilterContext, nullptr)) {}


	Filter& Filter::operator=(Filter&& rhs)
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	Filter::~Filter()
	{
		if (mFilterContext)
		{
			avfilter_free(mFilterContext);
		}
	}


	AVFilterContext*& Filter::operator*()
	{
		return mFilterContext;
	}


	const AVFilterContext* Filter::operator*() const
	{
		return mFilterContext;
	}


	void Filter::Link(Filter& consumer, unsigned int srcPad, unsigned int dstPad)
	{
		auto result = avfilter_link(**this, srcPad, *consumer, dstPad);
		Core::Assert(result == 0);
	}


	BufferSource::BufferSource(const AudioFrameFormat& format)
		: mFormat(format)
	{}


	void BufferSource::SendFrame(Frame frame)
	{
		auto result = av_buffersrc_add_frame(mFilterContext, *frame);
		Core::Assert(result == 0);
	}


	BufferSink::BufferSink() {}


	Core::Option<Frame> BufferSink::ReadFrame()
	{
		Frame frame;
		auto result = av_buffersink_get_frame(mFilterContext, *frame);
		switch (result)
		{
			case 0:
				return frame;
			case AVERROR(EAGAIN):
				return {};
			default:
				Core::Unreachable();
		}
	}


	Core::Option<Codec::Frame> BufferSink::PeekFrame()
	{
		Frame frame;
		auto result = av_buffersink_get_frame_flags(const_cast<AVFilterContext*>(mFilterContext), *frame, AV_BUFFERSINK_FLAG_PEEK);
		switch (result)
		{
			case 0: return frame;
			case AVERROR(EAGAIN): return {};
			default: Core::Unreachable();
		}
	}


	bool BufferSink::OutputAvailable()
	{
		auto result = av_buffersink_get_frame_flags(const_cast<AVFilterContext*>(mFilterContext), nullptr, AV_BUFFERSINK_FLAG_PEEK);
		switch (result)
		{
			case 0: return true;
			case AVERROR(EAGAIN): return false;
			default: Core::Unreachable();
		}
	}


	uint64_t BufferSink::GetSampleRate() const
	{
		return av_buffersink_get_sample_rate(mFilterContext);
	}


	uint64_t BufferSink::GetSampleFormat() const
	{
		return av_buffersink_get_format(mFilterContext);
	}


	int BufferSink::GetChannelCount() const
	{
		return av_buffersink_get_channels(mFilterContext);
	}


	unsigned long long int BufferSink::GetChannelLayout() const
	{
		return av_buffersink_get_channel_layout(mFilterContext);
	}
}