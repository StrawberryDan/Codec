#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
/// Standard Library
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <map>
/// Strawberry Libraries
#include "Strawberry/Core/Mutex.hpp"
#include "Strawberry/Core/Option.hpp"
#include "Strawberry/Core/Collection/CircularBuffer.hpp"
/// Strawberry Codec
#include "Filter.hpp"
#include "Frame.hpp"
#include "Constants.hpp"


extern "C"
{
#include "libavfilter/avfilter.h"
}


namespace Strawberry::Codec
{
	class FilterGraph
	{
	public:
		FilterGraph(MediaType type);
		FilterGraph(const FilterGraph&) = delete;
		FilterGraph& operator=(const FilterGraph&) = delete;
		FilterGraph(FilterGraph&&) = delete;
		FilterGraph& operator=(FilterGraph&&) = delete;
		~FilterGraph();


		Core::Option<BufferSource*>
		AddAudioInput(unsigned int index, uint64_t sampleRate, uint64_t sampleFormat,
					  uint64_t channelCount,
					  uint64_t channelLayout);
		BufferSource*               GetInput(unsigned int index);
		void                        RemoveInput(unsigned int index);
		Core::Option<BufferSink*>   AddOutput(unsigned int index, const std::string& args);
		BufferSink*                 GetOutput(unsigned int index);
		size_t                      GetInputCount() const;
		size_t                      GetOutputCount() const;


		std::vector<std::pair<unsigned int, BufferSource*>> GetInputPairs();
		std::vector<std::pair<unsigned int, BufferSink*>>   GetOutputPairs();


		Core::Option<Filter*> AddFilter(const std::string& filter, const std::string& name, const std::string& args);
		Core::Option<Filter*> GetFilter(const std::string& name);
		void                  RemoveFilter(const std::string& name);


		bool Configure();
		void Start();
		bool OutputAvailable(unsigned int index);


		void SendFrame(unsigned int inputIndex, Frame frame);
		Core::Option<Frame> RecvFrame(unsigned int outputIndex);


	protected:
		void                      Stop();


	private:
		std::atomic<bool>         mRunning;
		Core::Option<std::thread> mThread;
		void                      Run();
		std::atomic<bool>         mWarmingUp;



	private:
		std::mutex mGraphInteractionMutex;

		MediaType mMediaType;
		AVFilterGraph* mFilterGraph;
		std::unordered_map<std::string, Filter> mFilters;
		std::map<unsigned int, BufferSource> mInputs;
		std::map<unsigned int, BufferSink> mOutputs;
		std::vector<Core::Option<Frame>> mNextOutputs;

		using FrameBuffer = std::map<unsigned int, Core::Mutex<Core::Collection::DynamicCircularBuffer<Frame>>>;
		FrameBuffer mInputFrameBuffers;
		FrameBuffer mOutputFrameBuffers;


		std::atomic<bool> mConfigurationValid = false;
		std::atomic<bool> mConfigurationDirty = true;
	};
}
