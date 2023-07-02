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
	Filter::Filter(std::mutex* graphMutex)
			: mFilterContext(nullptr)
			, mGraphMutex(graphMutex) {
		Core::Assert(graphMutex != nullptr);
	}


	Filter::Filter(Filter&& rhs)
			: mFilterContext(std::exchange(rhs.mFilterContext, nullptr))
			, mGraphMutex(rhs.mGraphMutex){}


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
		avfilter_free(mFilterContext);
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
		std::unique_lock<std::mutex> lock(*mGraphMutex);
		auto result = avfilter_link(**this, srcPad, *consumer, dstPad);
		Core::Assert(result == 0);
	}


	BufferSource::BufferSource(std::mutex* graphMutex)
			: Filter(graphMutex) {}


	void BufferSource::SendFrame(Frame& frame)
	{
		auto result = av_buffersrc_add_frame(mFilterContext, *frame);
		Core::Assert(result == 0);
	}


	uint64_t BufferSource::GetSampleRate() const
	{
		return mSampleRate;
	}


	uint64_t BufferSource::GetSampleFormat() const
	{
		return mSampleFormat;
	}


	uint64_t BufferSource::GetChannelCount() const
	{
		return mChannelCount;
	}


	uint64_t BufferSource::GetChannelLayout() const
	{
		return mChannelLayout;
	}


	BufferSink::BufferSink(std::mutex* graphMutex)
			: Filter(graphMutex) {}


	Core::Option<Frame> BufferSink::ReadFrame()
	{
		Frame frame;
		auto result = av_buffersink_get_frame(mFilterContext, *frame);
		switch (result)
		{
			case 0: return frame;
			case AVERROR(EAGAIN): return {};
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