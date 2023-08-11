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
			result = mFrameResizer.ReadFrame(FrameResizer::Mode::WaitForFullFrames);
			if (result) return result;

			result = mResampler.ReadFrame();
			if (result)
			{
				mFrameResizer.SendFrame(result.Unwrap());
				continue;
			}

			if (mCurrentPosition < mCurrentTrackFrames.size())
			{
				if (mCurrentPosition == mCurrentTrackFrames.size() - 1 && mNextTracks.empty())
				{
					mEventBroadcaster.Broadcast(PlaybackEndedEvent{});
				}


				result = (mCurrentTrackFrames)[(mCurrentPosition)++];
				mResampler.SendFrame(result.Unwrap());
				continue;
			}

			if (!mNextTracks.empty())
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


		mNextTracks.push_back(track);
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

		if (index < mPreviousTracks.size())
		{
			mPreviousTracks.erase(mPreviousTracks.begin() + static_cast<long>(index));
		}
		else if (mCurrentTrack && index == mPreviousTracks.size())
		{
			mCurrentTrack.Reset();
			mCurrentTrackFrames.clear();
		}
		else if (!mCurrentTrack && index == mPreviousTracks.size())
		{
			mNextTracks.pop_front();
		}
		else
		{
			Core::Assert(index >= mPreviousTracks.size() + (mCurrentTrack.HasValue() ? 1 : 0));
			mNextTracks.erase(mNextTracks.begin() + static_cast<long>(index) - static_cast<long>(mPreviousTracks.size()) -
							  (mCurrentTrack.HasValue() ? 1 : 0));
		}

		SongRemovedEvent event
			{
				.index = index,
			};
		mEventBroadcaster.Broadcast(event);

		if (Length() == 0)
		{
			mEventBroadcaster.Broadcast(PlaybackEndedEvent{});
		}
	}


	std::shared_ptr<Core::IO::ChannelReceiver<Playlist::Event>> Playlist::CreateEventReceiver()
	{
		return mEventBroadcaster.CreateReceiver();
	}


	size_t Playlist::Length() const
	{
		return mPreviousTracks.size() + mNextTracks.size() + (mCurrentTrack.HasValue() ? 1 : 0);
	}


	Codec::Audio::FrameFormat Playlist::GetFrameFormat() const
	{
		return mFormat;
	}


	size_t Playlist::GetFrameSize() const
	{
		return mFrameSize;
	}


	void Playlist::GotoPrevTrack()
	{
		if (!mPreviousTracks.empty())
		{
			if (mCurrentTrack.HasValue())
			{
				mNextTracks.push_front(mCurrentTrack.Unwrap());
			}

			mCurrentTrack = mPreviousTracks[0];
			mCurrentTrackFrames = mCurrentTrack->loader();
			mPreviousTracks.pop_front();

			mEventBroadcaster.Broadcast(
				SongBeganEvent
					{
						.index        = mPreviousTracks.size(),
						.offset       = -1,
						.title = (mCurrentTrack)->title,
						.path  = (mCurrentTrack)->fileName,
					});
		}

		(mCurrentPosition) = 0;
	}


	void Playlist::GotoNextTrack()
	{
		if (!mNextTracks.empty())
		{
			if (mCurrentTrack.HasValue())
				mPreviousTracks.push_front(mCurrentTrack.Unwrap());


			mCurrentTrack = mNextTracks.front();
			mCurrentTrackFrames = (mCurrentTrack)->loader();
			mNextTracks.pop_front();
			(mCurrentPosition) = 0;

			mEventBroadcaster.Broadcast(
				SongBeganEvent
					{
						.index        = mPreviousTracks.size(),
						.offset       = 1,
						.title = (mCurrentTrack)->title,
						.path  = (mCurrentTrack)->fileName,
					});
		}
	}
}