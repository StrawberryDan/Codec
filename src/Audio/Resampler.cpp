#include <fmt/format.h>
#include "Codec/Audio/Resampler.hpp"
#include "Strawberry/Core/Utilities.hpp"


#include "Strawberry/Core/Assert.hpp"


extern "C"
{
#include "libavfilter/buffersink.h"
}



using Strawberry::Core::Assert;
using Strawberry::Core::Take;


namespace Strawberry::Codec::Audio
{
	Resampler::Resampler(FrameFormat targetFormat)
	{}


	void Resampler::SendFrame(Frame frame)
	{

	}


	Core::Option<Frame> Resampler::ReadFrame()
	{
		return {};
	}
}