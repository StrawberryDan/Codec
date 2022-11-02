#pragma once



#include <vector>
#include "Frame.hpp"
#include "Sample.hpp"



extern "C"
{
#include "libswresample/swresample.h"
#include "libavutil/channel_layout.h"
}



namespace Strawberry::Codec
{
	class Resampler
	{
	public:
	    Resampler();
	    explicit Resampler(const AVCodecParameters* codecParameters);
	    Resampler(const Resampler& other) = delete;
	    Resampler& operator=(const Resampler& other) = delete;
	    Resampler(Resampler&& other) noexcept ;
	    Resampler& operator=(Resampler&& other) noexcept ;
	    ~Resampler();

	    Frame Resample(const Frame& input);
	    std::vector<Frame> Resample(const std::vector<Frame>& input);

	private:
	    SwrContext* mSwrContext;
	};
}