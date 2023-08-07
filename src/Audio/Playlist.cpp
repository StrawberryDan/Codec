//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/Audio/Playlist.hpp"
// Codec
#include "Codec/MediaFile.hpp"
#include "Codec/MediaStream.hpp"
// Standard Library
#include <vector>



namespace Strawberry::Codec::Audio
{
	Playlist::Playlist(Audio::FrameFormat format, size_t sampleCount)
		: mFormat(format)
		, mFrameSize()
		, mResampler(format)
		, mFrameResizer(sampleCount)
	{}


	Core::Option<Frame> Playlist::ReadFrame()
	{
		Core::Option<Frame> result;

		while (true)
		{
			result = mFrameResizer.ReadFrame(FrameResizer::Mode::WaitForFullFrames);
			if (result) return result;

			result = mResampler.ReadFrame();
			if (result)
			{
				mFrameResizer.SendFrame(result.Unwrap());
				continue;
			}

			if (auto frames = mCurrentTrack.Lock(); !frames->empty())
			{
				result = (*frames)[mCurrentPosition++];
				mResampler.SendFrame(result.Unwrap());
				continue;
			}

			if (auto nextTracks = mNextTracks.Lock(); !nextTracks->empty())
			{
				*mCurrentTrack.Lock() = nextTracks->front()();
				nextTracks->pop_front();
				mCurrentPosition = 0;
				continue;
			}

			return Core::NullOpt;
		}
	}


	void Playlist::EnqueueFile(const std::string path)
	{
		mNextTracks.Lock()->push_back([=]() -> std::vector<Frame>
		{
			auto file = MediaFile::Open(path);
			if (!file) return {};

			auto channel = file->GetBestStream(MediaType::Audio);
			if (!channel) return {};

			std::vector<Packet> packets;
			while (auto packet = channel->Read())
			{
				packets.emplace_back(packet.Unwrap());
			}

			std::vector<Frame> frames;
			auto decoder = channel->GetDecoder();
			for (auto& packet : packets)
			{
				for (auto frame : decoder.DecodePacket(std::move(packet)))
				{
					frames.emplace_back(std::move(frame));
				}
			}

			return frames;
		});
	}
}