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


	Playlist::~Playlist()
	{
		StopLoading(true);
	}


	Core::Option<Frame> Playlist::ReadFrame()
	{
		Core::Option<Frame> result;


		while (true)
		{
			if (mReadingThread && !mShouldRead)
			{
				StopLoading(false);
			}
			else if (!mReadingThread && mCurrentTrackFrames.Lock()->empty() && mCurrentTrack)
			{
				StartLoading(mCurrentTrack->loader);
			}


			result = mFrameResizer.ReadFrame(FrameResizer::Mode::WaitForFullFrames);
			if (result) return result;

			result = mResampler.ReadFrame();
			if (result)
			{
				mFrameResizer.SendFrame(result.Unwrap());
				continue;
			}


			auto currentFrames = mCurrentTrackFrames.Lock();
			if (mCurrentPosition < currentFrames->size())
			{
				if (mCurrentPosition == currentFrames->size() - 1 && mNextTracks.empty() && !mReadingThread)
				{
					mEventBroadcaster.Broadcast(PlaybackEndedEvent{});
				}


				result = (*currentFrames)[mCurrentPosition++];
				mResampler.SendFrame(result.Unwrap());
				continue;
			}
			else if (mReadingThread)
			{
				std::this_thread::yield();
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


	Core::Option<size_t> Playlist::EnqueueFile(const std::string& path, std::any associatedData)
	{
		Track track;

		track.loader = [this, path](Core::Mutex<FrameBuffer>& frames) mutable
		{
			Core::Assert(frames.Lock()->empty());


			auto file = MediaFile::Open(path);
			if (!file)
			{
				mShouldRead = false;
				return;
			}

			auto channel = file->GetBestStream(MediaType::Audio);
			if (!channel)
			{
				mShouldRead = false;
				return;
			}


			auto decoder = channel->GetDecoder();
			for (auto packet = channel->Read(); mShouldRead && packet; packet = channel->Read())
			{
				decoder.Send(packet.Unwrap());
				for (auto frame : decoder.Receive())
				{
					frames.Lock()->emplace_back(std::move(frame));
				}
			}

			mShouldRead = false;
		};


		track.associatedData = associatedData;


		mNextTracks.push_back(track);
		SongAddedEvent songAddedEvent
			{
				.index = Length() - 1,
				.associatedData = associatedData,
			};
		mEventBroadcaster.Broadcast(songAddedEvent);

		return Length() - 1;
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
			StopLoading(true);
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


	size_t Playlist::GetCurrentTrackIndex() const
	{
		return mPreviousTracks.size();
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
			StopLoading(true);
			if (mCurrentTrack.HasValue())
			{
				mNextTracks.push_front(mCurrentTrack.Unwrap());
			}

			mCurrentTrack = mPreviousTracks[0];
			mPreviousTracks.pop_front();

			mEventBroadcaster.Broadcast(
				SongBeganEvent
					{
						.index          = mPreviousTracks.size(),
						.offset         = -1,
						.associatedData = mCurrentTrack->associatedData,
					});
		}

		mCurrentPosition = 0;
	}


	void Playlist::GotoNextTrack()
	{
		if (!mNextTracks.empty())
		{
			StopLoading(true);
			if (mCurrentTrack.HasValue())
				mPreviousTracks.push_front(mCurrentTrack.Unwrap());


			mCurrentTrack = mNextTracks.front();
			mNextTracks.pop_front();
			mCurrentPosition = 0;

			mEventBroadcaster.Broadcast(
				SongBeganEvent
					{
						.index          = mPreviousTracks.size(),
						.offset         = 1,
						.associatedData = mCurrentTrack->associatedData,
					});
		}
	}


	void Playlist::StartLoading(Playlist::TrackLoader loader)
	{
		StopLoading(true);
		Core::Assert(!mShouldRead);
		Core::Assert(!mReadingThread.HasValue());


		mShouldRead = true;
		mReadingThread.Emplace([this, loader](){ loader(mCurrentTrackFrames); });
	}


	void Playlist::StopLoading(bool clearFrames)
	{
		if (mReadingThread)
		{
			mShouldRead = false;
			mReadingThread->join();
			mReadingThread.Reset();
		}


		if (clearFrames)
			mCurrentTrackFrames.Lock()->clear();
	}
}