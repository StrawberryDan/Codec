#include "Codec/Audio/Encoder.hpp"
#include "Codec/Audio/Mixer.hpp"
#include "Codec/MediaFile.hpp"
#include "Codec/Muxer.hpp"
#include <Strawberry/Core/Timing/ScopedTimer.hpp>
#include <iostream>


extern "C"
{
#include "libavfilter/avfilter.h"
}


using namespace Strawberry::Codec;
using namespace Strawberry;


std::vector<Audio::Frame> DecodeAudioFile(const std::string& filePath)
{
	MediaFile file   = MediaFile::Open(filePath).Unwrap();
	auto      stream = file.GetBestStream(MediaType::Audio);
	Core::Assert(stream);

	std::vector<Packet> packets;
	while (auto packet = stream->Read())
	{
		packets.push_back(packet.Unwrap());
	}


	std::vector<Audio::Frame> frames;
	Audio::Decoder            decoder = stream->GetDecoder();
	for (auto& packet: packets)
	{
		decoder.Send(std::move(packet));
		auto someFrames = decoder.Receive();
		for (auto& f: someFrames)
		{
			frames.push_back(std::move(f));
		}
	}

	return frames;
}


void AudioMixing()
{
	Core::ScopedTimer timer("Audio Mixing");


	std::vector<Audio::Frame> frames[] = {
		// DecodeAudioFile("data/pd.wav"),
		// DecodeAudioFile("data/girigiri.wav"),
		// DecodeAudioFile("data/dcl.wav"),
		DecodeAudioFile("data/cotn.flac"),
	};


	Audio::Mixer                                             mixer(Audio::FrameFormat(48000, AV_SAMPLE_FMT_DBL, AV_CHANNEL_LAYOUT_STEREO), 1024);
	std::vector<std::shared_ptr<Audio::Mixer::InputChannel>> mixerChannels;
	for (auto& x: frames) mixerChannels.push_back(mixer.CreateInputChannel());


	std::vector<Packet> packets;
	for (int i = 0; i < std::extent_v<decltype(frames)>; i++)
	{
		for (auto& frame: frames[i])
		{
			mixerChannels[i]->EnqueueFrame(frame);
		}
	}


	packets.clear();
	Audio::Encoder encoder(AV_CODEC_ID_OPUS, AV_CHANNEL_LAYOUT_STEREO);
	while (!mixer.IsEmpty())
	{
		encoder.Send(mixer.ReadFrame());
		auto somePackets = encoder.Receive();
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
}


int main()
{
	av_log_set_level(AV_LOG_WARNING);

	AudioMixing();
}
