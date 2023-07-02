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


//	FilterGraph::FilterGraph(FilterGraph&& rhs)
//	{
//		auto wasRunning = *rhs.mRunning.Lock();
//		if (wasRunning) rhs.Stop();
//
//		mMediaType = rhs.mMediaType;
//		mFilterGraph = std::exchange(rhs.mFilterGraph, nullptr);
//		mInputs = std::move(rhs.mInputs);
//		mOutputs = std::move(rhs.mOutputs);
//		mInputFrameBuffers = std::move(rhs.mInputFrameBuffers);
//		mOutputFrameBuffers = std::move(rhs.mOutputFrameBuffers);
//
//		if (wasRunning) Start();
//	}


//	FilterGraph& FilterGraph::operator=(FilterGraph&& rhs)
//	{
//		if (this != &rhs)
//		{
//			std::destroy_at(this);
//			std::construct_at(this, std::move(rhs));
//		}
//
//		return *this;
//	}


	FilterGraph::~FilterGraph()
	{
		if (mFilterGraph)
		{
			Stop();
			mInputs.clear();
			mOutputs.clear();
			mFilters.clear();
			avfilter_graph_free(&mFilterGraph);
		}
	}


	Core::Option<BufferSource*>
	FilterGraph::AddAudioInput(unsigned int index, uint64_t sampleRate, uint64_t sampleFormat,
							   uint64_t channelCount,
							   uint64_t channelLayout)
	{
		mConfigurationDirty = true;

		BufferSource filter(&mGraphInteractionMutex);
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

		std::string args;
		if (channelLayout != 0)
		{
			args = fmt::format("sample_rate={}:sample_fmt={}:channel_layout={}:channels={}", sampleRate,
									sampleFormat, channelLayout, channelCount);
		}
		else
		{
			args = fmt::format("sample_rate={}:sample_fmt={}:channels={}", sampleRate,
							   sampleFormat, channelCount);
		}

		auto result = avfilter_graph_create_filter(&*filter, filterPtr, fmt::format("input-{}", mInputs.size()).c_str(),
												   args.c_str(), nullptr, mFilterGraph);
		if (result < 0) return {};
		filter.mSampleRate = sampleRate;
		filter.mSampleFormat = sampleFormat;
		filter.mChannelCount = channelCount;
		filter.mChannelLayout = channelLayout;

		mInputs.emplace(index, std::move(filter));
		mInputFrameBuffers.emplace_back();
		return &mInputs.at(index);
	}


	BufferSource* FilterGraph::GetInput(unsigned int index)
	{
		if (!mInputs.contains(index)) return nullptr;
		return &mInputs.at(index);
	}


	void FilterGraph::RemoveInput(unsigned int index)
	{
		mInputs.erase(index);
		mConfigurationDirty = true;
	}


	Core::Option<BufferSink*> FilterGraph::AddOutput(unsigned int index, const std::string& args)
	{
		mConfigurationDirty = true;

		BufferSink filter(&mGraphInteractionMutex);
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

		mOutputs.emplace(index, std::move(filter));
		mOutputFrameBuffers.emplace_back();
		return &mOutputs.at(index);
	}


	BufferSink* FilterGraph::GetOutput(unsigned int index)
	{
		if (!mOutputs.contains(index)) return nullptr;
		return &mOutputs.at(index);
	}


	size_t FilterGraph::GetInputCount() const
	{
		return mInputs.size();
	}


	size_t FilterGraph::GetOutputCount() const
	{
		return mOutputs.size();
	}


	std::vector<std::pair<unsigned int, BufferSource*>> FilterGraph::GetInputPairs()
	{
		std::vector<std::pair<unsigned int, BufferSource*>> result;
		for (auto& [i, buffer] : mInputs)
		{
			result.emplace_back(i, &buffer);
		}
		return result;
	}


	std::vector<std::pair<unsigned int, BufferSink*>> FilterGraph::GetOutputPairs()
	{
		std::vector<std::pair<unsigned int, BufferSink*>> result;
		for (auto& [i, buffer] : mOutputs)
		{
			result.emplace_back(i, &buffer);
		}
		return result;
	}


	Core::Option<Filter*>
	FilterGraph::AddFilter(const std::string& filterId, const std::string& name, const std::string& args)
	{
		std::unique_lock<std::mutex> lock(mGraphInteractionMutex);

		mConfigurationDirty = true;

		Core::Assert(!name.starts_with("input-"));
		Core::Assert(!name.starts_with("output-"));

		Filter filter(&mGraphInteractionMutex);

		auto filterPtr = avfilter_get_by_name(filterId.c_str());
		if (!filterPtr) return {};
		auto result = avfilter_graph_create_filter(&*filter, filterPtr, name.c_str(), args.c_str(), nullptr,
												   mFilterGraph);
		if (result < 0) return {};

		mFilters.emplace(name, std::move(filter));
		return &mFilters.at(name);
	}


	Core::Option<Filter*> FilterGraph::GetFilter(const std::string& name)
	{
		std::unique_lock<std::mutex> lock(mGraphInteractionMutex);

		if (mFilters.contains(name))
		{
			return &mFilters.at(name);
		}

		return {};
	}


	void FilterGraph::RemoveFilter(const std::string& name)
	{
		mFilters.erase(name);
	}


	bool FilterGraph::Configure()
	{
		if (mConfigurationDirty)
		{
			std::unique_lock<std::mutex> lock(mGraphInteractionMutex);
			Stop();
			auto result = avfilter_graph_config(mFilterGraph, nullptr);
			Start();
			mConfigurationValid = result >= 0;
			mConfigurationDirty = false;
		}

		return mConfigurationValid;
	}


	void FilterGraph::SendFrame(unsigned int inputIndex, Frame frame)
	{
		mInputFrameBuffers[inputIndex].Lock()->Push(std::move(frame));
	}


	Core::Option<Frame> FilterGraph::RecvFrame(unsigned int outputIndex)
	{
		if (!Configure()) return {};

		auto result = mOutputFrameBuffers[outputIndex].Lock()->Pop();
		while (!result)
		{
			std::this_thread::yield();
			result = mOutputFrameBuffers[outputIndex].Lock()->Pop();
		}

		return result;
	}


	void FilterGraph::Run()
	{
		while (mRunning)
		{
			for (auto [i, source] : GetInputPairs())
			{
				auto frame = mInputFrameBuffers[i].Lock()->Pop();
				if (frame)
				{
					if (mConfigurationValid && !mConfigurationDirty)
					{
						Core::Assert(source->GetSampleRate()    == (*frame)->sample_rate);
						Core::Assert(source->GetSampleFormat()  == (*frame)->format);
						Core::Assert(source->GetChannelCount()  == (*frame)->ch_layout.nb_channels);
						Core::Assert(source->GetChannelLayout() == (*frame)->channel_layout);
						source->SendFrame(*frame);
					}
				}
			}

			mWarmingUp = false;

			for (auto [i, sink] : GetOutputPairs())
			{
				if (mConfigurationValid && !mConfigurationDirty)
				{
					auto frame = sink->ReadFrame();

					if (frame)
					{
						mOutputFrameBuffers[i].Lock()->Push(frame.Take());
					}
				}
			}
		}
	}


	bool FilterGraph::OutputAvailable(unsigned int index)
	{
		if (!Configure()) return false;
		Core::Assert(mRunning);

		if (mWarmingUp) return true;

		for (auto& buffer : mInputFrameBuffers)
		{
			if (buffer.Lock()->Size() > 0) return true;
		}

		std::unique_lock<std::mutex> lock(mGraphInteractionMutex);
		Frame frame;
		auto result = av_buffersink_get_frame_flags(const_cast<AVFilterContext*>(*mOutputs.at(index)), *frame, AV_BUFFERSINK_FLAG_PEEK);
		switch (result)
		{
			case 0: return true;
			case AVERROR(EAGAIN): break;
			default: Core::Unreachable();
		}

		if (mOutputFrameBuffers[index].Lock()->Size() > 0) return true;

		return false;
	}


	void FilterGraph::Start()
	{
		if (!mRunning)
		{
			mRunning = true;
			mThread.Emplace(std::bind(&FilterGraph::Run, this));
			mWarmingUp = true;
		}
	}


	void FilterGraph::Stop()
	{
		if (mRunning)
		{
			Core::Assert(mThread.HasValue());
			mRunning = false;
			Core::Assert(mThread->joinable());
			mThread->join();
			mThread.Reset();
		}
	}
}