#include <fmt/format.h>
#include "Codec/Audio/FrameResizer.hpp"



#include "Strawberry/Core/Assert.hpp"



namespace Strawberry::Codec::Audio
{
	FrameResizer::FrameResizer(size_t sampleCount)
	{}



	void FrameResizer::SendFrame(Frame frame)
	{

	}


	Core::Option<Frame> FrameResizer::ReadFrame()
	{
		return {};
	}
}