#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
/// Standard Library
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
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
		FilterGraph(FilterGraph&&);
		FilterGraph& operator=(FilterGraph&&);
		~FilterGraph();


		Core::Option<Filter*> AddInput(const std::string& args);
		Filter*               GetInput(unsigned int index);
		Core::Option<Filter*> AddOutput(const std::string& args);
		Filter*               GetOutput(unsigned int index);
		size_t                GetInputCount() const;
		size_t                GetOutputCount() const;


		Core::Option<Filter*> AddFilter(const std::string& filter, const std::string& name, const std::string& args);
		Core::Option<Filter*> GetFilter(const std::string& name);


		void Configure();
		void Start();


		void SendFrame(unsigned int inputIndex, Frame frame);
		Core::Option<Frame> RecvFrame(unsigned int outputIndex);


	protected:
		void                      Stop();


	private:
		Core::Mutex<bool>         mRunning;
		Core::Option<std::thread> mThread;
		void                      Run();



	private:
		MediaType mMediaType;
		AVFilterGraph* mFilterGraph;
		std::unordered_map<std::string, Filter> mFilters;
		std::vector<Filter> mInputs;
		std::vector<Filter> mOutputs;

		using FrameBuffer = std::vector<Core::Mutex<Core::Collection::DynamicCircularBuffer<Frame>>>;
		FrameBuffer mInputFrameBuffers;
		FrameBuffer mOutputFrameBuffers;
	};
}
