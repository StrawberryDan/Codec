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

	public:
		Playlist(Audio::FrameFormat format, size_t sampleCount);


		Core::Option<Frame>            ReadFrame();


		void                           EnqueueFile(const std::string path);


		void                           GotoPrevTrack();
		void                           GotoNextTrack();


	private:
		using TrackLoader = std::function<std::vector<Frame>()>;


		const Audio::FrameFormat                        mFormat;
		const size_t                                    mFrameSize;

		Resampler                                       mResampler;
		FrameResizer                                    mFrameResizer;

		Core::Mutex<std::uint64_t>                      mCurrentPosition = Core::Mutex<uint64_t>(0);
		Core::Mutex<std::deque<TrackLoader>>            mPreviousTracks;
		Core::Mutex<Core::Option<TrackLoader>>          mCurrentTrack;
		Core::Mutex<std::vector<Frame>>                 mCurrentTrackFrames;
		Core::Mutex<std::deque<TrackLoader>>            mNextTracks;
	};
}
