#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "FrameFormat.hpp"
// C++ Standard
#include <utility>
#include <vector>


extern "C"
{
#include "libavcodec/avcodec.h"
}


namespace Strawberry::Codec::Audio
{
	class Frame
	{
	public:
		static Frame Allocate();
		static Frame Silence(const FrameFormat& format, size_t samples);

	public:
		Frame();
		Frame(const Frame& other);
		Frame& operator=(const Frame& other);
		Frame(Frame&& other) noexcept;
		Frame& operator=(Frame&& other) noexcept;
		~Frame();


		[[nodiscard]] FrameFormat GetFormat() const;
		[[nodiscard]] size_t      GetChannelCount() const;
		[[nodiscard]] size_t      GetNumSamples() const;
		[[nodiscard]] size_t      GetSampleSize() const;
		[[nodiscard]] bool        IsFormatPlanar() const;
		[[nodiscard]] double      GetDuration() const;


		void                                  Append(const Frame& other);
		[[nodiscard]] std::pair<Frame, Frame> Split(size_t pos) const;
		[[nodiscard]] Frame                   Mix(const Frame& other) const;


		void Multiply(double multiplier);


		inline AVFrame* operator*()
		{
			return mFrame;
		}


		inline const AVFrame* operator*() const
		{
			return mFrame;
		}


		inline AVFrame* operator->()
		{
			return mFrame;
		}


		inline const AVFrame* operator->() const
		{
			return mFrame;
		}

	private:
		explicit Frame(AVFrame* frame);

	private:
		AVFrame* mFrame;
	};
} // namespace Strawberry::Codec::Audio
