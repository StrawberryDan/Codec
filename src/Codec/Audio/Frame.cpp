#include "Codec/Audio/Frame.hpp"


// Strawberry Core
#include "Strawberry/Core/Math/Rational.hpp"
#include "Strawberry/Core/Assert.hpp"
// Standard Library
#include <algorithm>


extern "C"
{
#include <libavutil/samplefmt.h>
}


namespace Strawberry::Codec::Audio
{
	Frame Frame::Allocate()
	{
		Frame frame;
		frame.mFrame = av_frame_alloc();
		return frame;
	}


	Frame Frame::Silence(const FrameFormat& format, size_t samples)
	{
		int result;

		Frame frame   = Frame::Allocate();
		frame->format = format.GetSampleFormat();
		result        = av_channel_layout_copy(&frame->ch_layout, format.GetChannels());
		Core::Assert(result == 0);
		frame->sample_rate = format.GetSampleRate();
		frame->nb_samples  = samples;

		result = av_frame_get_buffer(*frame, 0);
		Core::Assert(result == 0);

		result = av_samples_set_silence(frame->data, 0, frame->nb_samples, frame->ch_layout.nb_channels, static_cast<AVSampleFormat>(frame->format));
		Core::Assert(result == 0);
		Core::Assert(frame.GetFormat() == format);
		Core::Assert(frame.GetNumSamples() == samples);

		return frame;
	}


	Frame::Frame()
		: mFrame(nullptr) {}


	Frame::Frame(const Frame& other)
		: mFrame(other.mFrame ? av_frame_clone(other.mFrame) : nullptr)
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
		mFrame = std::exchange(other.mFrame, nullptr);
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


	FrameFormat Frame::GetFormat() const
	{
		return {mFrame->sample_rate, static_cast<AVSampleFormat>(mFrame->format), &mFrame->ch_layout};
	}


	size_t Frame::GetChannelCount() const
	{
		return mFrame->ch_layout.nb_channels;
	}


	size_t Frame::GetNumSamples() const
	{
		return static_cast<size_t>(mFrame->nb_samples);
	}


	size_t Frame::GetSampleSize() const
	{
		return av_get_bytes_per_sample(static_cast<AVSampleFormat>(mFrame->format));
	}


	bool Frame::IsFormatPlanar() const
	{
		return av_sample_fmt_is_planar(static_cast<AVSampleFormat>(mFrame->format));
	}


	double Frame::GetDuration() const
	{
		return (mFrame->nb_samples * Core::Math::Rational(1, mFrame->sample_rate)).Evaluate();
	}


	void Frame::Append(const Frame& other)
	{
		AVFrame* newFrame     = av_frame_alloc();
		newFrame->format      = mFrame->format;
		newFrame->nb_samples  = mFrame->nb_samples + other->nb_samples;
		newFrame->ch_layout   = mFrame->ch_layout;
		newFrame->sample_rate = mFrame->sample_rate;
		auto error            = av_frame_get_buffer(newFrame, 1);
		Core::Assert(error == 0);

		av_samples_copy(newFrame->data, mFrame->data, 0, 0, mFrame->nb_samples, mFrame->ch_layout.nb_channels, static_cast<AVSampleFormat>(mFrame->format));
		av_samples_copy(
			newFrame->data,
			other->data,
			mFrame->nb_samples,
			0,
			other->nb_samples,
			other->ch_layout.nb_channels,
			static_cast<AVSampleFormat>(other->format));

		Core::Assert(newFrame->nb_samples == mFrame->nb_samples + other->nb_samples);
		std::destroy_at(this);
		mFrame = newFrame;
	}


	std::pair<Frame, Frame> Frame::Split(size_t pos) const
	{
		pos = std::clamp<size_t>(pos, 0, mFrame->nb_samples);

		AVFrame* frames[2]     = {av_frame_alloc(), av_frame_alloc()};
		frames[0]->format      = mFrame->format;
		frames[1]->format      = mFrame->format;
		frames[0]->ch_layout   = mFrame->ch_layout;
		frames[1]->ch_layout   = mFrame->ch_layout;
		frames[0]->nb_samples  = static_cast<int>(pos);
		frames[1]->nb_samples  = static_cast<int>(mFrame->nb_samples - pos);
		frames[0]->sample_rate = mFrame->sample_rate;
		frames[1]->sample_rate = mFrame->sample_rate;

		int error;
		error = av_frame_get_buffer(frames[0], 0);
		Core::Assert(error == 0);
		error = av_frame_make_writable(frames[0]);
		Core::Assert(error == 0);
		error = av_frame_get_buffer(frames[1], 0);
		Core::Assert(error == 0);
		error = av_frame_make_writable(frames[1]);
		Core::Assert(error == 0);

		av_samples_copy(frames[0]->data, mFrame->data, 0, 0, pos, mFrame->ch_layout.nb_channels, static_cast<AVSampleFormat>(mFrame->format));
		av_samples_copy(
			frames[1]->data,
			mFrame->data,
			0,
			static_cast<int>(pos),
			mFrame->nb_samples - static_cast<int>(pos),
			mFrame->ch_layout.nb_channels,
			static_cast<AVSampleFormat>(mFrame->format));

		Core::Assert(frames[0]->nb_samples + frames[1]->nb_samples == mFrame->nb_samples);
		return {Frame(frames[0]), Frame(frames[1])};
	}


	template<typename T> requires (std::integral<T> || std::floating_point<T>)
	static void MixArrays(T* a, T* b, size_t count)
	{
		for (int i = 0; i < count; i++)
		{
			*(a + i) = *(a + i) + *(b + i);
		}
	}


	template<typename T> requires (std::integral<T> || std::floating_point<T>)
	static void MixArrays(void* a, void* b, size_t count)
	{
		MixArrays(reinterpret_cast<T*>(a), reinterpret_cast<T*>(b), count);
	}


	Frame Frame::Mix(const Frame& other) const
	{
		Core::Assert(GetFormat() == other.GetFormat());
		const size_t samplesToCopy = std::min(GetNumSamples(), other.GetNumSamples());

		Frame        result;
		Frame*       bigger = &result;
		const Frame* smaller;
		if (GetNumSamples() > other.GetNumSamples())
		{
			result  = (*this);
			smaller = &other;
		}
		else
		{
			result  = other;
			smaller = this;
		}


		switch (mFrame->format)
		{
			case AV_SAMPLE_FMT_U8:
				MixArrays<uint8_t>((*bigger)->data[0], (*smaller)->data[0], samplesToCopy * GetChannelCount());
				break;
			case AV_SAMPLE_FMT_S16:
				MixArrays<int16_t>((*bigger)->data[0], (*smaller)->data[0], samplesToCopy * GetChannelCount());
				break;
			case AV_SAMPLE_FMT_S32:
				MixArrays<int32_t>((*bigger)->data[0], (*smaller)->data[0], samplesToCopy * GetChannelCount());
				break;
			case AV_SAMPLE_FMT_S64:
				MixArrays<int64_t>((*bigger)->data[0], (*smaller)->data[0], samplesToCopy * GetChannelCount());
				break;
			case AV_SAMPLE_FMT_FLT:
				MixArrays<float>((*bigger)->data[0], (*smaller)->data[0], samplesToCopy * GetChannelCount());
				break;
			case AV_SAMPLE_FMT_DBL:
				MixArrays<double>((*bigger)->data[0], (*smaller)->data[0], samplesToCopy * GetChannelCount());
				break;

			case AV_SAMPLE_FMT_U8P:
				for (auto i = 0; i < mFrame->ch_layout.nb_channels; ++i) MixArrays<uint8_t>((*bigger)->data[i], (*smaller)->data[i], samplesToCopy);
				break;
			case AV_SAMPLE_FMT_S16P:
				for (auto i = 0; i < mFrame->ch_layout.nb_channels; ++i) MixArrays<int16_t>((*bigger)->data[i], (*smaller)->data[i], samplesToCopy);
				break;
			case AV_SAMPLE_FMT_S32P:
				for (auto i = 0; i < mFrame->ch_layout.nb_channels; ++i) MixArrays<int32_t>((*bigger)->data[i], (*smaller)->data[i], samplesToCopy);
				break;
			case AV_SAMPLE_FMT_S64P:
				for (auto i = 0; i < mFrame->ch_layout.nb_channels; ++i) MixArrays<int64_t>((*bigger)->data[i], (*smaller)->data[i], samplesToCopy);
				break;
			case AV_SAMPLE_FMT_FLTP:
				for (auto i = 0; i < mFrame->ch_layout.nb_channels; ++i) MixArrays<float>((*bigger)->data[i], (*smaller)->data[i], samplesToCopy);
				break;
			case AV_SAMPLE_FMT_DBLP:
				for (auto i = 0; i < mFrame->ch_layout.nb_channels; ++i) MixArrays<double>((*bigger)->data[i], (*smaller)->data[i], samplesToCopy);
				break;
			default:
				Core::Unreachable();
		}

		return result;
	}


	template<typename T>
	static void MultiplyTypes(float multiplier, void* data, size_t count)
	{
		T* cast = reinterpret_cast<T*>(data);
		for (int i = 0; i < count; i++)
		{
			cast[i] = multiplier * cast[i];
		}
	}


	void Frame::Multiply(double multiplier)
	{
		for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
		{
			switch (GetFormat().GetSampleFormat())
			{
				case AV_SAMPLE_FMT_U8:
					MultiplyTypes<uint8_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(uint8_t));
					break;
				case AV_SAMPLE_FMT_S16:
					MultiplyTypes<int16_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(int16_t));
					break;
				case AV_SAMPLE_FMT_S32:
					MultiplyTypes<int32_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(int32_t));
					break;
				case AV_SAMPLE_FMT_FLT:
					MultiplyTypes<float>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(float));
					break;
				case AV_SAMPLE_FMT_DBL:
					MultiplyTypes<double>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(double));
					break;
				case AV_SAMPLE_FMT_U8P:
					MultiplyTypes<uint8_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(uint8_t));
					break;
				case AV_SAMPLE_FMT_S16P:
					MultiplyTypes<int16_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(int16_t));
					break;
				case AV_SAMPLE_FMT_S32P:
					MultiplyTypes<int32_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(int32_t));
					break;
				case AV_SAMPLE_FMT_FLTP:
					MultiplyTypes<float>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(float));
					break;
				case AV_SAMPLE_FMT_DBLP:
					MultiplyTypes<double>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(double));
					break;
				case AV_SAMPLE_FMT_S64:
					MultiplyTypes<int64_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(int64_t));
					break;
				case AV_SAMPLE_FMT_S64P:
					MultiplyTypes<int64_t>(multiplier, mFrame->data[i], mFrame->linesize[i] / sizeof(int64_t));
					break;
				case AV_SAMPLE_FMT_NB:
				case AV_SAMPLE_FMT_NONE:
					Core::Unreachable();
			}
		}
	}


	Frame::Frame(AVFrame* frame)
		: mFrame(frame) {}
} // namespace Strawberry::Codec::Audio
