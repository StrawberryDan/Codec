#include "Codec/Frame.hpp"



#include "Core/Assert.hpp"



extern "C"
{
#include <libavutil/avutil.h>
}



namespace Strawberry::Codec
{
	Frame::Frame()
		: mFrame(av_frame_alloc())
	{
		Core::Assert(mFrame != nullptr);
	}



	Frame::Frame(const Frame& other)
		: mFrame(av_frame_clone(other.mFrame))
	{
		int error = av_frame_make_writable(mFrame);
		Core::Assert(error == 0);
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
		av_frame_move_ref(mFrame, other.mFrame);
		Core::Assert(mFrame != nullptr);
	}



	Frame& Frame::operator=(Frame&& other) noexcept
	{
		if (this != &other)
		{
			av_frame_unref(mFrame);
			av_frame_move_ref(mFrame, other.mFrame);
		}

		return (*this);
	}



	Frame::~Frame()
	{
		av_frame_free(&mFrame);
	}



	void Frame::Append(const Frame& other)
	{
		AVFrame* newFrame		= av_frame_alloc();
		newFrame->format		= mFrame->format;
		newFrame->nb_samples	= mFrame->nb_samples + other->nb_samples;
		newFrame->ch_layout		= mFrame->ch_layout;
		auto error 				= av_frame_get_buffer(newFrame, 1);
		Core::Assert(error == 0);

		av_samples_copy(newFrame->data, mFrame->data,
						0, 0,
						mFrame->nb_samples, mFrame->ch_layout.nb_channels,
						static_cast<AVSampleFormat>(mFrame->format));
		av_samples_copy(newFrame->data, other.mFrame->data,
						mFrame->nb_samples, 0,
						other.mFrame->nb_samples, other.mFrame->ch_layout.nb_channels,
						static_cast<AVSampleFormat>(other.mFrame->format));

		std::destroy_at(this);
		mFrame = newFrame;
	}



	std::pair<Frame, Frame> Frame::Split(size_t pos) const
	{
		pos = std::clamp<size_t>(pos, 0, mFrame->nb_samples);

		int error = 0;

		AVFrame* frames[2] = { av_frame_alloc(), av_frame_alloc() };

		frames[0]->format		= mFrame->format;
		frames[1]->format		= mFrame->format;
		frames[0]->ch_layout	= mFrame->ch_layout;
		frames[1]->ch_layout	= mFrame->ch_layout;
		frames[0]->nb_samples	= pos;
		frames[1]->nb_samples	= mFrame->nb_samples - pos;
		error = av_frame_get_buffer(frames[0], 0);
		Core::Assert(error == 0);
		error = av_frame_make_writable(frames[0]);
		Core::Assert(error == 0);
		error = av_frame_get_buffer(frames[1], 0);
		Core::Assert(error == 0);
		error = av_frame_make_writable(frames[1]);
		Core::Assert(error == 0);

		av_samples_copy(frames[0]->data, mFrame->data,
						0, 0,
						pos, mFrame->ch_layout.nb_channels,
						static_cast<AVSampleFormat>(mFrame->format));
		av_samples_copy(frames[1]->data, mFrame->data,
						0, pos,
						mFrame->nb_samples - pos, mFrame->ch_layout.nb_channels,
						static_cast<AVSampleFormat>(mFrame->format));

		return { Frame(frames[0]), Frame(frames[1]) };
	}



	Frame::Frame(AVFrame* frame)
		: mFrame(frame)
	{}
}