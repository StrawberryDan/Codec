#pragma once



#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"



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
		Packet(const uint8_t* data, size_t len);
		Packet(const Packet& other);
		Packet& operator=(const Packet& other);
		Packet(Packet&& other) noexcept ;
		Packet& operator=(Packet&& other) noexcept ;
		~Packet();

		inline operator bool() const { return mAVPacket && mAVPacket->data && mAVPacket->size > 0; }

		inline       AVPacket* operator*()       { return mAVPacket; }
		inline const AVPacket* operator*() const { return mAVPacket; }

		inline       AVPacket* operator->()       { return mAVPacket; }
		inline const AVPacket* operator->() const { return mAVPacket; }

		Core::IO::DynamicByteBuffer AsBytes() const;

	private:
		AVPacket* mAVPacket;
	};
}
