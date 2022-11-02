#pragma once



#include <array>
#include <vector>
#include <cstdint>
#include "Packet.hpp"
#include "sodium.h"



namespace Strawberry::Codec
{
	class SodiumEncrypter
	{
	public:
		using Key             = std::array<uint8_t, crypto_secretbox_NONCEBYTES>;
		using Nonce           = std::array<uint8_t, crypto_secretbox_KEYBYTES>;
		using Encrypted       = std::vector<uint8_t>;
		using EncryptedPacket = std::pair<Nonce, Encrypted>;



	public:
	    explicit SodiumEncrypter(Key key);

	    EncryptedPacket Encrypt(Nonce nonce, const Packet& packet);
	private:
	    Key mKey;
	};
}
