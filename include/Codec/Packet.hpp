#pragma once



extern "C"
{
#include "libavformat/avformat.h"
}



namespace Strawberry::Codec
{
	class Packet
	{
	public:
	    Packet();
	    Packet(const Packet& other);
	    Packet& operator=(const Packet& other);
	    Packet(Packet&& other) noexcept ;
	    Packet& operator=(Packet&& other) noexcept ;
	    ~Packet();

	    inline       AVPacket* operator*()       { return mAVPacket; }
	    inline const AVPacket* operator*() const { return mAVPacket; }

	    inline       AVPacket* operator->()       { return mAVPacket; }
	    inline const AVPacket* operator->() const { return mAVPacket; }
	private:
	    AVPacket* mAVPacket;
	};
}
