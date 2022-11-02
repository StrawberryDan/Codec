#include "Codec/Packet.hpp"



namespace Strawberry::Codec
{
	Packet::Packet()
	    : mAVPacket(nullptr)
	{
	    mAVPacket = av_packet_alloc();
	}



	Packet::Packet(const Packet& other)
	    : mAVPacket(nullptr)
	{
	    mAVPacket = av_packet_clone(other.mAVPacket);
	}



	Packet& Packet::operator=(const Packet& other)
	{
	    if (this == &other) return (*this);
	    if (mAVPacket) av_packet_free(&mAVPacket);
	    mAVPacket = av_packet_clone(other.mAVPacket);
	    return (*this);
	}



	Packet::Packet(Packet&& other)
	 noexcept     : Packet()
	{
	    av_packet_move_ref(mAVPacket, other.mAVPacket);
	}



	Packet& Packet::operator=(Packet&& other)
	 noexcept {
	    av_packet_move_ref(mAVPacket, other.mAVPacket);
	    return (*this);
	}



	Packet::~Packet()
	{
	    if (mAVPacket) av_packet_free(&mAVPacket);
	}
}