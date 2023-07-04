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
#include "AudioFrameFormat.hpp"


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


		Core::Option<InputFilter*>
		AddInputAudioBuffer(unsigned int index, AudioFrameFormat format);


		InputFilter*                 GetInput(unsigned int index);
		void                         RemoveInput(unsigned int index);
		Core::Option<BufferSink*>  AddOutput(unsigned int index);
		BufferSink*                GetOutput(unsigned int index);
		size_t                       GetInputCount() const;
		size_t                       GetOutputCount() const;


		std::vector<std::pair<unsigned int, InputFilter*>> GetInputPairs();
		std::vector<std::pair<unsigned int, BufferSink*>>   GetOutputPairs();


		Core::Option<Filter*> AddFilter(const std::string& filter, const std::string& name, const std::string& args);
		Core::Option<Filter*> GetFilter(const std::string& name);
		void                  RemoveFilter(const std::string& name);


		bool Configure();
		bool OutputAvailable(unsigned int index);


		void SendFrame(unsigned int inputIndex, Frame frame);
		Core::Option<Frame> RecvFrame(unsigned int outputIndex);


	private:
		MediaType                                             mMediaType;
		AVFilterGraph*                                        mFilterGraph;
		std::unordered_map<std::string, Filter>               mFilters;
		std::map<unsigned int, std::unique_ptr<InputFilter>>  mInputs;
		std::map<unsigned int, std::unique_ptr<BufferSink>>   mOutputs;


		bool mConfigurationValid = false;
		bool mConfigurationDirty = true;
	};
}
