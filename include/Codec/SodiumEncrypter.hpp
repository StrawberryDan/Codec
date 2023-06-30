#pragma once



#include <array>
#include <vector>
#include <cstdint>
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "sodium.h"



namespace Strawberry::Codec
{
	class SodiumEncrypter
	{
	public:
		using Key             = std::array<uint8_t, crypto_secretbox_KEYBYTES>;
		using Nonce           = std::array<uint8_t, crypto_secretbox_NONCEBYTES>;
		using Encrypted       = Core::IO::DynamicByteBuffer;
		using EncryptedPacket = std::pair<Nonce, Encrypted>;



	public:
	    explicit SodiumEncrypter(Key key);

	    EncryptedPacket Encrypt(Nonce nonce, const Core::IO::DynamicByteBuffer& packet);



	private:
	    Key mKey;
	};
}
