#pragma once



#include "Sample.hpp"
#include <vector>



extern "C"
{
#include "libavcodec/avcodec.h"
}



namespace Strawberry::Codec
{
	class Frame
	{
	public:
	    Frame();
	    Frame(const Frame& other);
	    Frame& operator=(const Frame& other);
	    Frame(Frame&& other) noexcept ;
	    Frame& operator=(Frame&& other) noexcept ;
	    ~Frame();


	    inline       AVFrame* operator*()        { return mFrame; }
	    inline const AVFrame* operator*()  const { return mFrame; }
	    inline       AVFrame* operator->()       { return mFrame; }
	    inline const AVFrame* operator->() const { return mFrame; }

	    Samples GetSamples() const;

	private:
	    AVFrame* mFrame;
	};
}