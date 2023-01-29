#include "Codec/SodiumEncrypter.hpp"



#include "Core/Assert.hpp"



namespace Strawberry::Codec
{
	using namespace Strawberry::Core;



	SodiumEncrypter::SodiumEncrypter(Key key)
	    : mKey(key)
	{
	    auto result = sodium_init();
	    Assert(result >= 0);
	}



	SodiumEncrypter::EncryptedPacket SodiumEncrypter::Encrypt(Nonce nonce, const Packet& packet)
	{
	    Encrypted ciphertext;
	    ciphertext.resize(crypto_secretbox_MACBYTES + packet->size);
	    auto result = crypto_secretbox_easy(ciphertext.data(), packet->data, packet->size, nonce.data(), mKey.data());
	    Assert(result >= 0);
	    return {nonce, ciphertext};
	}
}