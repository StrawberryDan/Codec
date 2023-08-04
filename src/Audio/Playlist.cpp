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

			if (!mFrames.empty())
			{
				result = mFrames.front();
				mFrames.pop_front();
				mResampler.SendFrame(result.Unwrap());
				continue;
			}

			if (!mNextTracks.empty())
			{
				auto newFrames = mNextTracks.front()();
				mNextTracks.pop_front();
				for (auto& frame : newFrames) mFrames.emplace_back(std::move(frame));
				continue;
			}

			return Core::NullOpt;
		}
	}


	void Playlist::EnqueueFile(const std::string path)
	{
		mNextTracks.push_back([=]() -> std::vector<Frame>
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