#pragma once


#include "AudioFrameFormat.hpp"
#include "Frame.hpp"
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


	class InputFilter
		: public Filter
	{
	public:
		virtual ~InputFilter() {}

		virtual void SendFrame(Codec::Frame frame) = 0;
	};


	class BufferSource
			: public InputFilter
	{
		friend class FilterGraph;


	public:
		explicit BufferSource(const AudioFrameFormat& format);


		virtual void SendFrame(Codec::Frame frame) override;


		const AudioFrameFormat& FrameFormat() const { return mFormat; }


	private:
		AudioFrameFormat mFormat;
	};


	class BufferSink
		: public Filter
	{
	public:
		explicit BufferSink();


		Core::Option<Codec::Frame> ReadFrame();
		Core::Option<Codec::Frame> PeekFrame();
		bool OutputAvailable();


		uint64_t GetSampleRate() const;
		uint64_t GetSampleFormat() const;
		int GetChannelCount() const;
		unsigned long long int GetChannelLayout() const;
	};
}