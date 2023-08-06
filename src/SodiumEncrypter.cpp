#include "Codec/SodiumEncrypter.hpp"



#include "Strawberry/Core/Util/Assert.hpp"



namespace Strawberry::Codec
{
	using namespace Strawberry::Core;



	SodiumEncrypter::SodiumEncrypter(Key key)
		: mKey(key)
	{
		auto result = sodium_init();
		Assert(result >= 0);
	}



	SodiumEncrypter::EncryptedPacket SodiumEncrypter::Encrypt(Nonce nonce, const Core::IO::DynamicByteBuffer& packet)
	{
		auto ciphertext = Core::IO::DynamicByteBuffer::Zeroes(crypto_secretbox_MACBYTES + packet.Size());
		auto result = crypto_secretbox_easy(ciphertext.Data(), packet.Data(), packet.Size(), nonce.data(), mKey.data());
		Assert(result >= 0);
		return {nonce, ciphertext};
	}
}