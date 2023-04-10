#include "Codec/Frame.hpp"



#include "Core/Assert.hpp"



namespace Strawberry::Codec
{
	Frame::Frame()
			: mFrame(nullptr)
	{
		mFrame = av_frame_alloc();
		Core::Assert(mFrame != nullptr);
	}



	Frame::Frame(const Frame& other)
		: mFrame(av_frame_clone(other.mFrame))
	{
		Core::Assert(other.mFrame != nullptr);
		Core::Assert(mFrame != nullptr);
	}



	Frame& Frame::operator=(const Frame& other)
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, other);
		}

		return (*this);
	}



	Frame::Frame(Frame&& other) noexcept
		: Frame()
	{
		av_frame_unref(mFrame);
		av_frame_move_ref(mFrame, other.mFrame);
		Core::Assert(mFrame != nullptr);
		other.mFrame = nullptr;
	}



	Frame& Frame::operator=(Frame&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return (*this);
	}



	Frame::~Frame()
	{
		av_frame_free(&mFrame);
	}
}