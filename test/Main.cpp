#include <iostream>
#include "Codec/AudioFile.hpp"
#include "Codec/OpusEncoder.hpp"
#include "Codec/Muxer.hpp"
#include "Codec/SodiumEncrypter.hpp"



using namespace Strawberry::Codec;



int main()
{
    av_log_set_level(AV_LOG_DEBUG);

    AudioFile file("data/music.mp3");
    OpusEncoder encoder;
	std::vector<Frame> frames;
    while (!file.IsEof())
    {
        auto frame = file.ReadFrame();
		if (frame)
		{
			frames.push_back(frame.Unwrap());
		}
    }


	std::vector<Packet> packets;
	for (const auto& frame : frames)
	{
		auto somePackets = encoder.Encode(frame);
		for (const auto p : somePackets)
		{
			packets.push_back(p);
		}
	}


    SodiumEncrypter::Key key;
    crypto_secretbox_keygen(key.data());
    std::vector<SodiumEncrypter::EncryptedPacket> encrypted;
    SodiumEncrypter encrypter(key);
    for (const auto& packet : packets)
    {
        encrypted.push_back(encrypter.Encrypt({ 0 }, packet));
    }


    Muxer muxer("output.opus");
    muxer.OpenStream(encoder.Parameters());
    muxer.WriteHeader();
    for (auto& packet : packets)
    {
        muxer.WritePacket(packet);
    }
    muxer.WriteTrailer();
    muxer.Flush();

    return 0;
}