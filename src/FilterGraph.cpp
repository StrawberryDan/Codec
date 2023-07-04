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
			mInputs.clear();
			mOutputs.clear();
			mFilters.clear();
			avfilter_graph_free(&mFilterGraph);
		}
	}


	Core::Option<InputFilter*>
	FilterGraph::AddInputAudioBuffer(unsigned int index, AudioFrameFormat format)
	{
		mConfigurationDirty = true;

		auto filter = std::make_unique<BufferSource>(format);
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
		if (format.channelLayout != 0)
		{
			args = fmt::format("sample_rate={}:sample_fmt={}:channel_layout={}:channels={}", format.sampleRate,
							   format.sampleFormat, format.channelLayout, format.channels.nb_channels);
		}
		else
		{
			args = fmt::format("sample_rate={}:sample_fmt={}:channels={}", format.sampleRate,
							   format.sampleFormat, format.channels.nb_channels);
		}

		auto result = avfilter_graph_create_filter(&**filter, filterPtr, fmt::format("input-{}", mInputs.size()).c_str(),
												   args.c_str(), nullptr, mFilterGraph);
		if (result < 0) return {};

		mInputs.emplace(index, std::move(filter));
		return mInputs.at(index).get();
	}


	InputFilter* FilterGraph::GetInput(unsigned int index)
	{
		if (!mInputs.contains(index)) return nullptr;
		return mInputs.at(index).get();
	}


	void FilterGraph::RemoveInput(unsigned int index)
	{
		mInputs.erase(index);
		mConfigurationDirty = true;
	}


	Core::Option<BufferSink*> FilterGraph::AddOutput(unsigned int index)
	{
		mConfigurationDirty = true;

		BufferSink filter;
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
												   fmt::format("output-{}", mInputs.size()).c_str(), "",
												   nullptr, mFilterGraph);
		if (result < 0) return {};

		mOutputs.emplace(index, std::make_unique<BufferSink>(std::move(filter)));
		return mOutputs.at(index).get();
	}


	BufferSink* FilterGraph::GetOutput(unsigned int index)
	{
		if (!mOutputs.contains(index)) return nullptr;
		return mOutputs.at(index).get();
	}


	size_t FilterGraph::GetInputCount() const
	{
		return mInputs.size();
	}


	size_t FilterGraph::GetOutputCount() const
	{
		return mOutputs.size();
	}


	std::vector<std::pair<unsigned int, InputFilter*>> FilterGraph::GetInputPairs()
	{
		std::vector<std::pair<unsigned int, InputFilter*>> result;
		for (auto& [i, buffer] : mInputs)
		{
			result.emplace_back(i, buffer.get());
		}
		return result;
	}


	std::vector<std::pair<unsigned int, BufferSink*>> FilterGraph::GetOutputPairs()
	{
		std::vector<std::pair<unsigned int, BufferSink*>> result;
		for (auto& [i, buffer] : mOutputs)
		{
			result.emplace_back(i, buffer.get());
		}
		return result;
	}


	Core::Option<Filter*>
	FilterGraph::AddFilter(const std::string& filterId, const std::string& name, const std::string& args)
	{
		mConfigurationDirty = true;

		Core::Assert(!name.starts_with("input-"));
		Core::Assert(!name.starts_with("output-"));

		Filter filter;

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
			auto result = avfilter_graph_config(mFilterGraph, nullptr);
			mConfigurationValid = result >= 0;
			mConfigurationDirty = false;
		}

		return mConfigurationValid;
	}


	void FilterGraph::SendFrame(unsigned int inputIndex, Frame frame)
	{
		auto configure = Configure();
		Core::Assert(configure);
		mInputs.at(inputIndex)->SendFrame(std::move(frame));
	}


	Core::Option<Frame> FilterGraph::RecvFrame(unsigned int outputIndex)
	{
		if (!Configure()) return {};
		return mOutputs.at(outputIndex)->ReadFrame();
	}


	bool FilterGraph::OutputAvailable(unsigned int index)
	{
		if (!Configure()) return false;
		if (mOutputs.at(index)->OutputAvailable()) return true;
		return false;
	}
}