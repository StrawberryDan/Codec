#include "Codec/Frame.hpp"



#include "Standard/Assert.hpp"



namespace Strawberry::Codec
{
	Frame::Frame()
	    : mFrame(nullptr)
	{
	    mFrame = av_frame_alloc();
	    Assert(mFrame != nullptr);
	}



	Frame::Frame(const Frame& other)
	    : mFrame(nullptr)
	{
	    Assert(other.mFrame != nullptr);
	    mFrame = av_frame_clone(other.mFrame);
	    Assert(mFrame != nullptr);
	}



	Frame& Frame::operator=(const Frame& other)
	{
	    if (this == &other) return (*this);
	    Assert(other.mFrame != nullptr);
	    if (mFrame) av_frame_free(&mFrame);
	    mFrame = av_frame_clone(other.mFrame);
	    Assert(mFrame != nullptr);
	    return (*this);
	}



	Frame::Frame(Frame&& other) noexcept
	    : Frame()
	{
	    av_frame_unref(mFrame);
	    av_frame_move_ref(mFrame, other.mFrame);
	    Assert(mFrame != nullptr);
	    other.mFrame = nullptr;
	}



	Frame& Frame::operator=(Frame&& other) noexcept
	{
	    av_frame_unref(mFrame);
	    av_frame_move_ref(mFrame, other.mFrame);
	    Assert(mFrame != nullptr);
	    other.mFrame = nullptr;
	    return (*this);
	}



	Frame::~Frame()
	{
	    av_frame_free(&mFrame);
	}



	Samples Frame::GetSamples() const
	{
	    return {mFrame->data[0], mFrame->data[1], static_cast<size_t>(mFrame->nb_samples * sizeof(Sample))};
	}
}