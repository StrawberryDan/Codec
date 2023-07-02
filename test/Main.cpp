#include <iostream>
#include "Codec/AudioFile.hpp"
#include "Codec/Encoder.hpp"
#include "Codec/Muxer.hpp"
#include "Codec/SodiumEncrypter.hpp"
#include "Strawberry/Core/Logging.hpp"


extern "C"
{
#include "libavfilter/avfilter.h"
}



using namespace Strawberry::Codec;
using namespace Strawberry;


int main()
{
	av_log_set_level(AV_LOG_DEBUG);

	AudioFile file("data/selen.mp3");
	std::vector<Packet> packets;
	while (!file.IsEof())
	{
		auto packet = file.ReadPacket();
		if (packet)
		{
			packets.push_back(packet.Unwrap());
		}
	}


	std::vector<Frame> frames;
	Decoder decoder(file.GetCodec(), file.GetCodecParameters());
	for (const auto& packet: packets)
	{
		auto someFrames = decoder.DecodePacket(packet);
		for (auto& f: someFrames)
		{
			frames.push_back(std::move(f));
		}
	}


	packets.clear();
	Encoder encoder(AV_CODEC_ID_OPUS, AV_CHANNEL_LAYOUT_STEREO);
	for (auto& frame: frames)
	{
		auto somePackets = encoder.Encode(frame);
		for (const auto p: somePackets)
		{
			packets.push_back(p);
		}
	}


	Muxer muxer("output.opus");
	muxer.OpenStream(encoder.Parameters());
	muxer.WriteHeader();
	for (auto& packet: packets)
	{
		muxer.WritePacket(packet);
	}
	muxer.WriteTrailer();
	muxer.Flush();

	return 0;
}