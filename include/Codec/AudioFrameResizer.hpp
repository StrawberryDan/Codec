#pragma once



#include "Frame.hpp"
#include <stddef.h>
#include <vector>



namespace Strawberry::Codec
{
	class AudioFrameResizer
	{
	public:
							AudioFrameResizer(size_t sampleCount);



		std::vector<Frame>	Process(Frame frame);



	private:
		size_t BufferedSampleCount() const;



	private:
		const size_t		mSampleCount;
		std::vector<Frame>	mFrameBuffer;
	};
}
