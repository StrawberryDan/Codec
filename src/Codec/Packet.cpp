#include "Codec/Packet.hpp"


#include "Strawberry/Core/Assert.hpp"


namespace Strawberry::Codec
{
	Packet::Packet()
		: mAVPacket(av_packet_alloc()) {}


	Packet::Packet(const uint8_t* data, size_t len)
		: Packet()
	{
		auto avData = av_malloc(len);
		memcpy(avData, data, len);
		auto result = av_packet_from_data(mAVPacket, static_cast<uint8_t*>(avData), static_cast<int>(len));
		Core::Assert(result == 0);
	}


	Packet::Packet(const Packet& other)
		: mAVPacket(av_packet_clone(other.mAVPacket))
	{
		int error = av_packet_make_writable(mAVPacket);
		Core::Assert(error == 0);
	}


	Packet& Packet::operator=(const Packet& other)
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, other);
		}

		return *this;
	}


	Packet::Packet(Packet&& other) noexcept
		: Packet()
	{
		av_packet_move_ref(mAVPacket, other.mAVPacket);
	}


	Packet& Packet::operator=(Packet&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return (*this);
	}


	Packet::~Packet()
	{
		if (mAVPacket)
		{
			av_packet_free(&mAVPacket);
		}
	}


	Core::IO::DynamicByteBuffer Packet::AsBytes() const
	{
		return {mAVPacket->data, static_cast<size_t>(mAVPacket->size)};
	}
} // namespace Strawberry::Codec
