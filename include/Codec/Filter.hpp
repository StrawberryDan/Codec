#pragma once


#include <mutex>
#include "Strawberry/Core/Option.hpp"


extern "C"
{
#include "libavfilter/avfilter.h"
}


namespace Strawberry::Codec
{
	class Filter
	{
	public:
		Filter(std::mutex* graphMutex);
		Filter(const Filter&) = delete;
		Filter& operator=(const Filter&) = delete;
		Filter(Filter&&);
		Filter& operator=(Filter&&);
		~Filter();


		AVFilterContext*& operator*();
		const AVFilterContext* operator*() const;


		void Link(Filter& consumer, unsigned int srcPad, unsigned int dstPad);


	private:
		AVFilterContext* mFilterContext;
		std::mutex*      mGraphMutex;
	};


	class BufferSource
		: public Filter
	{
		friend class FilterGraph;


	public:
		explicit BufferSource(std::mutex* graphMutex);


		uint64_t GetSampleRate() const;
		uint64_t GetSampleFormat() const;
		uint64_t GetChannelCount() const;
		uint64_t GetChannelLayout() const;


	private:
		uint64_t mSampleRate;
		uint64_t mSampleFormat;
		uint64_t mChannelCount;
		uint64_t mChannelLayout;
	};



	class BufferSink
		: public Filter
	{
	public:
		explicit BufferSink(std::mutex* graphMutex);


		uint64_t GetSampleRate() const;
		uint64_t GetSampleFormat() const;
		uint64_t& GetChannelCount() const;
		uint64_t& GetChannelLayout() const;
	};
}