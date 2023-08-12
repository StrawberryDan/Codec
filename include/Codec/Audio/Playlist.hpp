#pragma once
//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/Audio/Frame.hpp"
#include "Codec/Audio/Resampler.hpp"
#include "Codec/Audio/FrameResizer.hpp"
// Core
#include "Strawberry/Core/IO/ChannelBroadcaster.hpp"
#include "Strawberry/Core/Util/Option.hpp"
#include "Strawberry/Core/Util/Variant.hpp"
// Standard Library
#include <any>
#include <vector>
#include <deque>
#include <functional>


//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Codec::Audio
{
	class Playlist
	{
	public:
		struct SongBeganEvent
		{
			/// The index of the new currently playing song
			size_t                    index;
			/// The difference in playlist index.
			int                       offset;
			/// The data associated with the new song
			std::any                  associatedData;
		};


		struct SongAddedEvent
		{
			/// The index where the song was inserted.
			size_t                    index;
			/// The data associated with the new song
			std::any                  associatedData;
		};


		struct SongRemovedEvent
		{
			/// The index where the song was inserted.
			size_t                    index;
		};


		struct PlaybackEndedEvent
		{

		};


		using Event = Core::Variant<
			SongBeganEvent,
			SongAddedEvent,
			SongRemovedEvent,
			PlaybackEndedEvent>;

		using EventReceiver = std::shared_ptr<Core::IO::ChannelReceiver<Playlist::Event>>;


	public:
		Playlist(Audio::FrameFormat format, size_t sampleCount);


		Core::Option<Frame>              ReadFrame();


		[[nodiscard]]
		Core::Option<size_t>             EnqueueFile(const std::string& path, std::any associatedData = {});


		void                             RemoveTrack(size_t index);


		EventReceiver                    CreateEventReceiver();


		[[nodiscard]]
		size_t                           GetCurrentTrackIndex() const;
		[[nodiscard]]
		size_t                           Length() const;
		[[nodiscard]]
		Codec::Audio::FrameFormat        GetFrameFormat() const;
		[[nodiscard]]
		size_t                           GetFrameSize() const;


		template <typename T>
		[[nodiscard]]
		T                                GetTrackAssociatedData(size_t index) const;
		template <typename T>
		void                             SetTrackAssociatedData(size_t index, T value);


		void                             GotoPrevTrack();
		void                             GotoNextTrack();


	private:
		using FrameBuffer = std::vector<Frame>;
		using TrackLoader = std::function<void(Core::Mutex<FrameBuffer>&)>;


		struct Track
		{
			TrackLoader               loader;
			std::any                  associatedData;
		};


		const Audio::FrameFormat      mFormat;
		const size_t                  mFrameSize;

		Resampler                     mResampler;
		FrameResizer                  mFrameResizer;

		std::uint64_t                 mCurrentPosition = 0;
		std::deque<Track>             mPreviousTracks;
		Core::Option<Track>           mCurrentTrack;
		Core::Mutex<FrameBuffer>      mCurrentTrackFrames;
		std::deque<Track>             mNextTracks;


		Core::IO::ChannelBroadcaster<Playlist::Event> mEventBroadcaster;
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
}
