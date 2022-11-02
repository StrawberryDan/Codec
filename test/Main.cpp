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
    Samples samples;
    while (!file.IsEof())
    {
        auto frame = file.ReadFrame();
        if (frame)
        {
            auto someSamples = frame->GetSamples();
            samples.Append(std::move(someSamples));
        }
    }


    std::vector<Packet> packets = encoder.Encode(samples);
    {
        auto final = encoder.Finish();
        packets.insert(packets.end(), final.begin(), final.end());
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