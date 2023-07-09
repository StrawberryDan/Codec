#include <iostream>
#include <Strawberry/Core/ScopedTimer.hpp>
#include "Codec/AudioFile.hpp"
#include "Codec/AudioMixer.hpp"
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


std::vector<Frame> DecodeAudioFile(const std::string& filePath)
{
	AudioFile file(filePath);
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

	return frames;
};


int main()
{
	Core::ScopedTimer timer("Test");


	av_log_set_level(AV_LOG_FATAL);


	std::vector<Frame> frames[] =
	{
		DecodeAudioFile("data/pd.wav"),
		DecodeAudioFile("data/girigiri.mp3"),
		DecodeAudioFile("data/dcl.wav"),
		DecodeAudioFile("data/cotn.flac"),
	};


	std::vector<Packet> packets;
	AudioMixer mixer(std::extent_v<decltype(frames)>);
	for (int i = 0; i < std::extent_v<decltype(frames)>; i++)
	{
		for (auto& frame : frames[i])
		{
			mixer.Send(i, frame);
		}
	}



	packets.clear();
	Encoder encoder(AV_CODEC_ID_OPUS, AV_CHANNEL_LAYOUT_STEREO);
	while (auto frame= mixer.ReceiveFrame())
	{
		auto somePackets = encoder.Encode(*frame);
		for (const auto p: somePackets)
		{
			packets.push_back(p);
		}
	}


	Muxer muxer("output.opus");
	muxer.OpenStream(encoder.Parameters());
	muxer.WriteHeader();
	int pts = 0;
	for (auto& packet: packets)
	{
		packet->pts = pts;
		pts += packet->duration;
		muxer.WritePacket(packet);
	}
	muxer.WriteTrailer();
	muxer.Flush();

	return 0;
}