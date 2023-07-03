#pragma once


#include <mutex>
#include "Strawberry/Core/Option.hpp"
#include "Frame.hpp"


extern "C"
{
#include "libavfilter/avfilter.h"
}


namespace Strawberry::Codec
{
	class Filter
	{
	public:
		Filter();
		Filter(const Filter&) = delete;
		Filter& operator=(const Filter&) = delete;
		Filter(Filter&&);
		Filter& operator=(Filter&&);
		~Filter();


		AVFilterContext*& operator*();
		const AVFilterContext* operator*() const;


		void Link(Filter& consumer, unsigned int srcPad, unsigned int dstPad);


	protected:
		AVFilterContext* mFilterContext;
	};


	class BufferSource
		: public Filter
	{
		friend class FilterGraph;


	public:
		explicit BufferSource();


		void SendFrame(Codec::Frame frame);


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
		explicit BufferSink();


		Core::Option<Codec::Frame> ReadFrame();
		bool OutputAvailable();


		uint64_t GetSampleRate() const;
		uint64_t GetSampleFormat() const;
		int GetChannelCount() const;
		unsigned long long int GetChannelLayout() const;
	};
}