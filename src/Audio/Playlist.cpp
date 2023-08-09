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
		  , mFrameResizer(sampleCount) {}


	Core::Option<Frame> Playlist::ReadFrame()
	{
		Core::Option<Frame> result;

		while (true)
		{
			auto currentPosition = mCurrentPosition.Lock();
			auto currentFrames = mCurrentTrackFrames.Lock();
			auto nextTracks = mNextTracks.Lock();


			result = mFrameResizer.ReadFrame(FrameResizer::Mode::WaitForFullFrames);
			if (result) return result;

			result = mResampler.ReadFrame();
			if (result)
			{
				mFrameResizer.SendFrame(result.Unwrap());
				continue;
			}

			if (!currentFrames->empty())
			{
				result = (*currentFrames)[(*currentPosition)++];
				mResampler.SendFrame(result.Unwrap());
				continue;
			}

			if (!nextTracks->empty())
			{
				GotoNextTrack();
				continue;
			}

			return Core::NullOpt;
		}
	}


	void Playlist::EnqueueFile(const std::string& path)
	{
		Track track;


		auto file = MediaFile::Open(path);
		if (!file) return;

		auto channel = file->GetBestStream(MediaType::Audio);
		if (!channel) return;


		track.title = channel->GetTitle();
		track.fileName = path;


		track.loader = [=]() mutable -> std::vector<Frame>
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
			for (auto& packet: packets)
			{
				decoder.Send(std::move(packet));
				for (auto frame: decoder.Receive())
				{
					frames.emplace_back(std::move(frame));
				}
			}

			return frames;
		};


		auto nextTracks = mNextTracks.Lock();
		nextTracks->push_back(track);
		SongAddedEvent songAddedEvent
			{
				.index = Length() - 1,
				.title=track.title,
				.path=track.fileName
			};
		mEventBroadcaster.Broadcast(songAddedEvent);
	}


	void Playlist::RemoveTrack(size_t index)
	{
		Core::Assert(index < Length());
		auto nextTracks = mNextTracks.Lock();
		auto currentTrack = mCurrentTrack.Lock();
		auto currentFrames = mCurrentTrackFrames.Lock();
		auto prevTracks = mPreviousTracks.Lock();

		if (index < prevTracks->size())
		{
			prevTracks->erase(prevTracks->begin() + static_cast<long>(index));
		}
		else if (*currentTrack && index == prevTracks->size())
		{
			currentTrack->Reset();
			currentFrames->clear();

		}
		else if (!*currentTrack && index == prevTracks->size())
		{
			nextTracks->pop_front();
		}
		else
		{
			Core::Assert(index >= prevTracks->size() + (currentTrack->HasValue() ? 1 : 0));
			nextTracks->erase(nextTracks->begin() + static_cast<long>(index) - static_cast<long>(prevTracks->size()) - (currentTrack->HasValue() ? 1 : 0));
		}

		SongRemovedEvent event
			{
				.index = index,
			};
		mEventBroadcaster.Broadcast(event);
	}


	std::shared_ptr<Core::IO::ChannelReceiver<Playlist::Event>> Playlist::CreateEventReceiver()
	{
		return mEventBroadcaster.CreateReceiver();
	}


	size_t Playlist::Length() const
	{
		auto nextTracks = mNextTracks.Lock();
		auto currentTrack = mCurrentTrack.Lock();
		auto prevTracks = mPreviousTracks.Lock();
		return prevTracks->size() + nextTracks->size() + (currentTrack->HasValue() ? 1 : 0);
	}


	void Playlist::GotoPrevTrack()
	{
		auto nextTracks = mNextTracks.Lock();
		auto currentTrack = mCurrentTrack.Lock();
		auto prevTracks = mPreviousTracks.Lock();
		auto currentTrackFrames = mCurrentTrackFrames.Lock();
		auto currentPosition = mCurrentPosition.Lock();

		if (!prevTracks->empty())
		{
			if (currentTrack->HasValue())
			{
				nextTracks->push_front(currentTrack->Unwrap());
			}

			*currentTrack = (*prevTracks)[0];
			*currentTrackFrames = (*currentTrack)->loader();
			prevTracks->pop_front();

			mEventBroadcaster.Broadcast(SongChangedEvent
											{
												.offset       = -1,
												.title = (*currentTrack)->title,
												.path  = (*currentTrack)->fileName,
											});
		}

		(*currentPosition) = 0;
	}


	void Playlist::GotoNextTrack()
	{
		auto nextTracks = mNextTracks.Lock();
		auto currentTrack = mCurrentTrack.Lock();
		auto prevTracks = mPreviousTracks.Lock();
		auto currentTrackFrames = mCurrentTrackFrames.Lock();
		auto currentPosition = mCurrentPosition.Lock();


		if (!nextTracks->empty())
		{
			if (currentTrack->HasValue())
				prevTracks->push_front(currentTrack->Unwrap());


			*currentTrack = nextTracks->front();
			*currentTrackFrames = (*currentTrack)->loader();
			nextTracks->pop_front();
			(*currentPosition) = 0;

			mEventBroadcaster.Broadcast(
				SongChangedEvent
					{
						.offset       = 1,
						.title = (*currentTrack)->title,
						.path  = (*currentTrack)->fileName,
					});
		}
	}
}