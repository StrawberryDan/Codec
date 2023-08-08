#pragma once
//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/Audio/Frame.hpp"
#include "Codec/Audio/Resampler.hpp"
#include "Codec/Audio/FrameResizer.hpp"
// Core
#include "Strawberry/Core/Util/Option.hpp"
#include "Strawberry/Core/Sync/Mutex.hpp"
#include "Strawberry/Core/IO/ChannelBroadcaster.hpp"
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
		struct SongChangedEvent
		{
			/// The difference in playlist index.
			int                       offset;
			Core::Option<std::string> newSongTitle;
			std::string               newSongPath;
		};


		using Event = Core::Variant<SongChangedEvent>;


	public:
		Playlist(Audio::FrameFormat format, size_t sampleCount);


		Core::Option<Frame>              ReadFrame();


		void                             EnqueueFile(const std::string& path);


		std::shared_ptr<Core::IO::ChannelReceiver<Playlist::Event>> CreateEventReceiver();


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


		const Audio::FrameFormat                        mFormat;
		const size_t                                    mFrameSize;

		Resampler                                       mResampler;
		FrameResizer                                    mFrameResizer;

		Core::Mutex<std::uint64_t>                      mCurrentPosition = Core::Mutex<uint64_t>(0);
		Core::Mutex<std::deque<Track>>                  mPreviousTracks;
		Core::Mutex<Core::Option<Track>>                mCurrentTrack;
		Core::Mutex<std::vector<Frame>>                 mCurrentTrackFrames;
		Core::Mutex<std::deque<Track>>                  mNextTracks;


		Core::IO::ChannelBroadcaster<Playlist::Event> mEventBroadcaster;
	};
}
