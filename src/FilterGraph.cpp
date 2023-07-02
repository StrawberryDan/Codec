//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/FilterGraph.hpp"
/// Lib Format
#include "fmt/format.h"
/// Standard Library
#include <memory>


extern "C"
{
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
}


namespace Strawberry::Codec
{
	FilterGraph::FilterGraph(MediaType type)
			: mMediaType(type), mFilterGraph(avfilter_graph_alloc()) {}


	FilterGraph::FilterGraph(FilterGraph&& rhs)
	{
		auto wasRunning = *rhs.mRunning.Lock();
		if (wasRunning) rhs.Stop();

		mMediaType = rhs.mMediaType;
		mFilterGraph = std::exchange(rhs.mFilterGraph, nullptr);
		mInputs = std::move(rhs.mInputs);
		mOutputs = std::move(rhs.mOutputs);
		mInputFrameBuffers = std::move(rhs.mInputFrameBuffers);
		mOutputFrameBuffers = std::move(rhs.mOutputFrameBuffers);

		if (wasRunning) Start();
	}


	FilterGraph& FilterGraph::operator=(FilterGraph&& rhs)
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	FilterGraph::~FilterGraph()
	{
		if (mFilterGraph)
		{
			Stop();
			avfilter_graph_free(&mFilterGraph);
		}
	}


	Core::Option<Filter*> FilterGraph::AddInput(const std::string& args)
	{
		Filter filter;
		const AVFilter* filterPtr = nullptr;
		switch (mMediaType)
		{
			case MediaType::Audio:
				filterPtr = avfilter_get_by_name("abuffer");
				break;
			case MediaType::Video:
				filterPtr = avfilter_get_by_name("buffer");
				break;
			default:
				Core::Unreachable();
		}
		auto result = avfilter_graph_create_filter(&*filter, filterPtr, fmt::format("input-{}", mInputs.size()).c_str(),
												   args.c_str(), nullptr, mFilterGraph);
		if (result < 0) return {};

#if NDEBUG
		Configure();
#endif // NDEBUG

		mInputs.emplace_back(std::move(filter));
		mInputFrameBuffers.emplace_back();
		return &mInputs.back();
	}


	Filter* FilterGraph::GetInput(unsigned int index)
	{
		return &mInputs[index];
	}


	Core::Option<Filter*> FilterGraph::AddOutput(const std::string& args)
	{
		Filter filter;
		const AVFilter* filterPtr = nullptr;
		switch (mMediaType)
		{
			case MediaType::Audio:
				filterPtr = avfilter_get_by_name("abuffersink");
				break;
			case MediaType::Video:
				filterPtr = avfilter_get_by_name("buffersink");
				break;
			default:
				Core::Unreachable();
		}
		auto result = avfilter_graph_create_filter(&*filter, filterPtr,
												   fmt::format("output-{}", mInputs.size()).c_str(), args.c_str(),
												   nullptr, mFilterGraph);
		if (result < 0) return {};

#if NDEBUG
		Configure();
#endif // NDEBUG

		mOutputs.emplace_back(std::move(filter));
		mOutputFrameBuffers.emplace_back();
		return &mOutputs.back();
	}


	Filter* FilterGraph::GetOutput(unsigned int index)
	{
		return &mOutputs[index];
	}


	size_t FilterGraph::GetInputCount() const
	{
		return mInputs.size();
	}


	size_t FilterGraph::GetOutputCount() const
	{
		return mOutputs.size();
	}


	Core::Option<Filter*>
	FilterGraph::AddFilter(const std::string& filterId, const std::string& name, const std::string& args)
	{
		Core::Assert(!name.starts_with("input-"));
		Core::Assert(!name.starts_with("output-"));

		Filter filter;

		auto filterPtr = avfilter_get_by_name(filterId.c_str());
		if (!filterPtr) return {};
		auto result = avfilter_graph_create_filter(&*filter, filterPtr, name.c_str(), args.c_str(), nullptr,
												   mFilterGraph);
		if (result < 0) return {};

#if NDEBUG
		Configure();
#endif // NDEBUG

		mFilters[name] = std::move(filter);
		return &mFilters[name];
	}


	Core::Option<Filter*> FilterGraph::GetFilter(const std::string& name)
	{
		if (mFilters.contains(name))
		{
			return &mFilters[name];
		}

		return {};
	}


	void FilterGraph::Configure()
	{
		auto result = avfilter_graph_config(mFilterGraph, nullptr);
		Core::Assert(result >= 0);
	}


	void FilterGraph::SendFrame(unsigned int inputIndex, Frame frame)
	{
		mInputFrameBuffers[inputIndex].Lock()->Push(std::move(frame));
	}


	Core::Option<Frame> FilterGraph::RecvFrame(unsigned int outputIndex)
	{
		Core::Assert(*mRunning.Lock());
		return mOutputFrameBuffers[outputIndex].Lock()->Pop();
	}


	void FilterGraph::Run()
	{
		while (*mRunning.Lock())
		{
			for (int i = 0; i < mInputs.size(); i++)
			{
				auto frame = mInputFrameBuffers[i].Lock()->Pop();
				if (frame)
				{
					auto result = av_buffersrc_add_frame(*mInputs[i], **frame);
					Core::Assert(result == 0);
				}
			}


			for (int i = 0; i < mOutputs.size(); i++)
			{
				Frame frame;
				auto result = av_buffersink_get_frame(*mOutputs[i], *frame);
				Core::Assert(result == 0 || AVERROR(EAGAIN));

				if (result == 0)
				{
					mOutputFrameBuffers[i].Lock()->Push(std::move(frame));
				}
			}
		}
	}


	void FilterGraph::Start()
	{
		auto running = mRunning.Lock();
		if (!*running)
		{
			*running = true;
			mThread.Emplace(std::bind(&FilterGraph::Run, this));
		}
	}


	void FilterGraph::Stop()
	{
		auto running = mRunning.Lock();
		if (*running)
		{
			*running = false;
			mThread->join();
			mThread.Reset();
		}
	}
}