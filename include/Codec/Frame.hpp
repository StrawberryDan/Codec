#pragma once



#include <vector>
#include <utility>



extern "C"
{
#include "libavcodec/avcodec.h"
}



namespace Strawberry::Codec
{
	class Frame
	{
	public:
		Frame();
		Frame(const Frame& other);
		Frame& operator=(const Frame& other);
		Frame(Frame&& other) noexcept ;
		Frame& operator=(Frame&& other) noexcept ;
		~Frame();


		void Append(const Frame& other);
		[[nodiscard]] std::pair<Frame, Frame> Split(size_t pos) const;


		inline       AVFrame* operator*()        { return mFrame; }
		inline const AVFrame* operator*()  const { return mFrame; }
		inline       AVFrame* operator->()       { return mFrame; }
		inline const AVFrame* operator->() const { return mFrame; }



	private:
		Frame(AVFrame* frame);



	private:
		AVFrame* mFrame;
	};
}