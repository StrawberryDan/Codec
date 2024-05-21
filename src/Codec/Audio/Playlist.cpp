//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/Audio/Playlist.hpp"
// Codec
#include "Codec/MediaFile.hpp"
#include "Codec/MediaStream.hpp"
// Standard Library
#include <vector>

namespace Strawberry::Codec::Audio::Playlist
{
	Playlist::Playlist(const Audio::FrameFormat& format, size_t sampleCount)
		: mFormat(format)
		, mFrameSize()
		, mResampler(format)
		, mFrameResizer(sampleCount)
		, mVolume(100, 100)
	{}

	Playlist::~Playlist()
	{
		StopLoading(true);
	}

	Core::Optional<Frame> Playlist::ReadFrame()
	{
		Core::Optional<Frame> result;


		while (true)
		{
			if (!mReadingThread && mCurrentTrackFrames.Lock()->empty() && mCurrentTrack) { StartLoading(mCurrentTrack->loader); }

			result = mFrameResizer.ReadFrame(FrameResizer::Mode::WaitForFullFrames);
			if (result)
			{
				mHasSentPlaybackEnded = false;
				result->Multiply(std::pow(2.0, mVolume.Evaluate()) - 1);
				return result;
			}

			result = mResampler.ReadFrame();
			if (result)
			{
				mFrameResizer.SendFrame(result.Unwrap());
				continue;
			}


			{
				auto currentFrames = mCurrentTrackFrames.Lock();
				if (!currentFrames->empty())
				{
					result = currentFrames->front();
					currentFrames->pop_front();
					mResampler.SendFrame(result.Unwrap());
					continue;
				}
				else if (mShouldRead && mReadingThread && mReadingActive)
				{
					std::this_thread::yield();
					continue;
				}
			}


			if (!mNextTracks.empty() && (!mCurrentTrack || !mCurrentTrack->repeat))
			{
				GotoNextTrack();
				continue;
			}
			else if (mCurrentTrack && mCurrentTrack->repeat)
			{
				continue;
			}


			if (!mHasSentPlaybackEnded)
			{
				mHasSentPlaybackEnded = true;
				Broadcast(PlaybackEndedEvent{});
			}


			return Core::NullOpt;
		}
	}

	Core::Optional<size_t> Playlist::EnqueueFile(const std::filesystem::path& path, const std::any& associatedData, bool repeat)
	{
		Track track;


		track.loader = [this, path](Core::Mutex<FrameBuffer>& frames) mutable {
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
			while (mShouldRead)
			{
				while (frames.Lock()->size() > 1024 && mShouldRead)
				{
					std::this_thread::yield();
				}

				auto packet = channel->Read();
				if (!packet)
				{
					mReadingActive = false;
					std::this_thread::yield();
					continue;
				}
				mReadingActive = true;
				decoder.Send(packet.Unwrap());
				for (auto frame : decoder.Receive()) { frames.Lock()->emplace_back(std::move(frame)); }
			}
		};


		track.associatedData = associatedData;
		track.repeat         = repeat;


		mNextTracks.push_back(track);

		Broadcast(SongAddedEvent{
			.index          = Length() - 1,
			.associatedData = associatedData,
		});

		return Length() - 1;
	}

	void Playlist::RemoveTrack(size_t index)
	{
		Core::Assert(index < Length());

		if (index < mPreviousTracks.size()) { mPreviousTracks.erase(mPreviousTracks.begin() + static_cast<long>(index)); }
		else if (mCurrentTrack && index == mPreviousTracks.size())
		{
			mCurrentTrack.Reset();
			StopLoading(true);
		}
		else if (!mCurrentTrack && index == mPreviousTracks.size()) { mNextTracks.pop_front(); }
		else
		{
			Core::Assert(index >= mPreviousTracks.size() + (mCurrentTrack.HasValue() ? 1 : 0));
			mNextTracks.erase(mNextTracks.begin() + static_cast<long>(index) - static_cast<long>(mPreviousTracks.size()) - (mCurrentTrack.HasValue() ? 1 : 0));
		}

		Broadcast(SongRemovedEvent{
			.index = index,
		});

		if (Length() == 0) { Broadcast(PlaybackEndedEvent{}); }
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
			if (mCurrentTrack.HasValue()) { mNextTracks.push_front(mCurrentTrack.Unwrap()); }

			mCurrentTrack = mPreviousTracks[0];
			mPreviousTracks.pop_front();

			Broadcast(SongBeganEvent{
				.index          = mPreviousTracks.size(),
				.offset         = -1,
				.repeating      = mCurrentTrack->repeat,
				.associatedData = mCurrentTrack->associatedData,
			});
		}
	}

	void Playlist::GotoNextTrack()
	{
		if (!mNextTracks.empty())
		{
			StopLoading(true);
			if (mCurrentTrack.HasValue()) mPreviousTracks.push_front(mCurrentTrack.Unwrap());


			mCurrentTrack = mNextTracks.front();
			mNextTracks.pop_front();

			Broadcast(SongBeganEvent{
				.index          = mPreviousTracks.size(),
				.offset         = 1,
				.associatedData = mCurrentTrack->associatedData,
			});
		}
	}

	void Playlist::StartLoading(const Playlist::TrackLoader& loader)
	{
		StopLoading(true);
		Core::Assert(!mShouldRead);
		Core::Assert(!mReadingThread.HasValue());


		mShouldRead = true;
		mReadingActive = true;
		mReadingThread.Emplace([this, loader]() { loader(mCurrentTrackFrames); });
	}

	void Playlist::StopLoading(bool clearFrames)
	{
		if (mReadingThread)
		{
			mShouldRead = false;
			mReadingThread->join();
			mReadingThread.Reset();
		}


		if (clearFrames) mCurrentTrackFrames.Lock()->clear();
	}

	bool Playlist::GetTrackRepeating(size_t index) const
	{
		if (index < mPreviousTracks.size()) { return mPreviousTracks[index].repeat; }

		index -= mPreviousTracks.size();
		if (index == 0) { return mCurrentTrack->repeat; }

		index -= 1;
		if (index < mNextTracks.size()) { return mNextTracks[index].repeat; }

		Core::Unreachable();
	}

	void Playlist::SetTrackRepeating(size_t index, bool repeating)
	{
		if (index < mPreviousTracks.size())
		{
			mPreviousTracks[index].repeat = repeating;
			return;
		}

		index -= mPreviousTracks.size();
		if (index == 0)
		{
			mCurrentTrack->repeat = repeating;
			return;
		}

		index -= 1;
		if (index < mNextTracks.size())
		{
			mNextTracks[index].repeat = repeating;
			return;
		}

		Core::Unreachable();
	}


	int Playlist::GetVolume() const
	{
		return mVolume.Numerator();
	}


	void Playlist::SetVolume(int volume)
	{
		mVolume.Numerator() = volume;
	}
} // namespace Strawberry::Codec::Audio::Playlist