#pragma once



//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "FrameFormat.hpp"
// C++ Standard
#include <vector>
#include <utility>



extern "C"
{
#include "libavcodec/avcodec.h"
}



namespace Strawberry::Codec::Audio
{
	class Frame
	{
	public:
		static Frame Silence(const FrameFormat& format, size_t samples);


	public:
		Frame();
		Frame(const Frame& other);
		Frame& operator=(const Frame& other);
		Frame(Frame&& other) noexcept ;
		Frame& operator=(Frame&& other) noexcept ;
		~Frame();


		FrameFormat GetFormat() const;
		size_t      GetNumSamples() const;


		void Append(const Frame& other);
		[[nodiscard]] std::pair<Frame, Frame> Split(size_t pos) const;
		void Mix(const Frame& other);


		inline       AVFrame* operator*()        { return mFrame; }
		inline const AVFrame* operator*()  const { return mFrame; }
		inline       AVFrame* operator->()       { return mFrame; }
		inline const AVFrame* operator->() const { return mFrame; }



	private:
		explicit Frame(AVFrame* frame);



	private:
		AVFrame* mFrame;
	};
}