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
			/// The title of the song.
			Core::Option<std::string> title;
			/// The file path of the song.
			std::string path;
		};


		struct SongAddedEvent
		{
			/// The index where the song was inserted.
			size_t                    index;
			/// The title of the song.
			Core::Option<std::string> title;
			/// The file path of the song.
			std::string path;
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


		void                             EnqueueFile(const std::string& path);


		void                             RemoveTrack(size_t index);


		EventReceiver                    CreateEventReceiver();


		size_t                           Length() const;
		Codec::Audio::FrameFormat        GetFrameFormat() const;
		size_t                           GetFrameSize() const;


		void                             GotoPrevTrack();
		void                             GotoNextTrack();


	private:
		using TrackLoader = std::function<std::vector<Frame>()>;


		struct Track
		{
			Core::Option<std::string> title;
			std::string               fileName;
			TrackLoader               loader;
		};


		const Audio::FrameFormat      mFormat;
		const size_t                  mFrameSize;

		Resampler                     mResampler;
		FrameResizer                  mFrameResizer;

		std::uint64_t                 mCurrentPosition = 0;
		std::deque<Track>             mPreviousTracks;
		Core::Option<Track>           mCurrentTrack;
		std::vector<Frame>            mCurrentTrackFrames;
		std::deque<Track>             mNextTracks;


		Core::IO::ChannelBroadcaster<Playlist::Event> mEventBroadcaster;
	};
}
