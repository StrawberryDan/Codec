#pragma once
//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/Audio/Frame.hpp"
#include "Codec/Audio/FrameResizer.hpp"
#include "Codec/Audio/Resampler.hpp"
// Core
#include "Strawberry/Core/IO/ChannelBroadcaster.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "Strawberry/Core/Math/Rational.hpp"
#include "Strawberry/Core/Sync/Mutex.hpp"
// Standard Library
#include <any>
#include <deque>
#include <filesystem>
#include <functional>
#include <thread>
#include <vector>

//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Codec::Audio::Playlist
{
	struct SongBeganEvent
	{
		/// The index of the new currently playing song
		size_t index;
		/// The difference in playlist index.
		int offset;
		/// Whether the track is repeating
		bool repeating;
		/// The data associated with the new song
		std::any associatedData;
	};


	struct SongAddedEvent
	{
		/// The index where the song was inserted.
		size_t index;
		/// The data associated with the new song
		std::any associatedData;
	};


	struct SongRemovedEvent
	{
		/// The index where the song was inserted.
		size_t index;
	};


	struct PlaybackEndedEvent {};


	using EventBroadcaster = Core::IO::ChannelBroadcaster<SongBeganEvent, SongAddedEvent, SongRemovedEvent, PlaybackEndedEvent>;
	using EventReceiver    = Core::IO::ChannelReceiver<SongBeganEvent, SongAddedEvent, SongRemovedEvent, PlaybackEndedEvent>;


	class Playlist : public EventBroadcaster
	{
	public:
		Playlist(const Audio::FrameFormat& format, size_t sampleCount);
		Playlist(const Playlist& rhs)            = delete;
		Playlist& operator=(const Playlist& rhs) = delete;
		Playlist(Playlist&& rhs)                 = delete;
		Playlist& operator=(Playlist&& rhs)      = delete;
		~Playlist();


		Core::Optional<Frame> ReadFrame();


		[[nodiscard]] Core::Optional<size_t> EnqueueFile(const std::filesystem::path& path, const std::any& associatedData = {}, bool repeat = false);


		void RemoveTrack(size_t index);


		[[nodiscard]] size_t                    GetCurrentTrackIndex() const;
		[[nodiscard]] size_t                    Length() const;
		[[nodiscard]] Codec::Audio::FrameFormat GetFrameFormat() const;
		[[nodiscard]] size_t                    GetFrameSize() const;


		template<typename T>
		[[nodiscard]] T GetTrackAssociatedData(size_t index) const;
		template<typename T>
		void SetTrackAssociatedData(size_t index, T value);


		bool GetTrackRepeating(size_t index) const;
		void SetTrackRepeating(size_t index, bool repeating);


		int  GetVolume() const;
		void SetVolume(int volume);


		void GotoPrevTrack();
		void GotoNextTrack();

	private:
		using FrameBuffer = std::deque<Frame>;
		using TrackLoader = std::function<void(Core::Mutex<FrameBuffer>&)>;

	private:
		void StartLoading(const TrackLoader& loader);
		void StopLoading(bool clearFrames);

	private:
		struct Track
		{
			TrackLoader loader;
			std::any    associatedData;
			bool        repeat;
		};


		Core::Math::Rational<int> mVolume = (100, 100);

		const Audio::FrameFormat mFormat;
		const size_t             mFrameSize;

		Resampler    mResampler;
		FrameResizer mFrameResizer;

		std::deque<Track>        mPreviousTracks;
		Core::Optional<Track>    mCurrentTrack;
		Core::Mutex<FrameBuffer> mCurrentTrackFrames;
		std::deque<Track>        mNextTracks;


		std::atomic<bool>           mShouldRead    = false;
		std::atomic<bool>           mReadingActive = false;
		Core::Optional<std::thread> mReadingThread;
		bool                        mHasSentPlaybackEnded = false;
	};


	template<typename T>
	T Playlist::GetTrackAssociatedData(size_t index) const
	{
		if (index < mPreviousTracks.size())
		{
			return std::any_cast<T>(mPreviousTracks[index].associatedData);
		}
		else if (index == mPreviousTracks.size() && mCurrentTrack)
		{
			return std::any_cast<T>(mCurrentTrack->associatedData);
		}
		else if (index > mPreviousTracks.size())
		{
			return std::any_cast<T>(mNextTracks[index - mPreviousTracks.size()].associatedData);
		}
		else
		{
			Core::Unreachable();
		}
	}


	template<typename T>
	void Playlist::SetTrackAssociatedData(size_t index, T value)
	{
		if (index < mPreviousTracks.size())
		{
			mPreviousTracks[index].associatedData = value;
		}
		else if (index == mPreviousTracks.size() && mCurrentTrack)
		{
			mCurrentTrack->associatedData = value;
		}
		else if (index == mPreviousTracks.size() && !mCurrentTrack && !mNextTracks.empty())
		{
			mNextTracks.front().associatedData = value;
		}
		else if (index > mPreviousTracks.size())
		{
			if (mCurrentTrack)
			{
				mNextTracks[index - mPreviousTracks.size()].associatedData = value;
			}
			else
			{
				mNextTracks[index - mPreviousTracks.size() - 1].associatedData = value;
			}
		}
		else
		{
			Core::Unreachable();
		}
	}
} // namespace Strawberry::Codec::Audio::Playlist
