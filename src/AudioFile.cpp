#include "Codec/AudioFile.hpp"
#include "Standard/Utilities.hpp"



#include <iostream>



using Strawberry::Standard::Assert;
using Strawberry::Standard::Take;
using Strawberry::Standard::Replace;



namespace Strawberry::Codec
{
	AudioFile::AudioFile(const std::string& path)
	    : mFile(nullptr)
	    , mCodec(nullptr)
	    , mStreamIndex{}
	    , mIsEof(false)
	    , mDecoder()
	    , mResampler()
	{
	    auto result = avformat_open_input(&mFile, path.c_str(), nullptr, nullptr);
	    Assert(result == 0);

	    result = avformat_find_stream_info(mFile, nullptr);
	    Assert(result == 0);

	    result = av_find_best_stream(mFile, AVMEDIA_TYPE_AUDIO, -1, -1, &mCodec, 0);
	    Assert(result >= 0);
	    Assert(mCodec != nullptr);
	    if (result >= 0)
	    {
	        mStreamIndex = result;
	    }

	    result = av_seek_frame(mFile, *mStreamIndex, 0, 0);
	    Assert(result >= 0);

	    mDecoder   = Decoder(mCodec, mFile->streams[*mStreamIndex]->codecpar);
	    mResampler = Resampler(mFile->streams[*mStreamIndex]->codecpar);
	}



	AudioFile::AudioFile(AudioFile&& other) noexcept
	    : mFile(Take(other.mFile))
	    , mCodec(Take(other.mCodec))
	    , mStreamIndex(Take(other.mStreamIndex))
	    , mIsEof(Replace(other.mIsEof, true))
	{

	}



	AudioFile& AudioFile::operator=(AudioFile&& other) noexcept
	{
	    mFile = Take(other.mFile);
	    return (*this);
	}



	AudioFile::~AudioFile()
	{
	    avformat_close_input(&mFile);
	}



	Standard::Option<Frame> AudioFile::ReadFrame()
	{
	    if (!mLeftoverFrames.empty())
	    {
	        auto frame = std::move(*mLeftoverFrames.begin());
	        mLeftoverFrames.erase(mLeftoverFrames.begin());
	        return frame;
	    }

	    if (IsEof())
	    {
	        return {};
	    }

	    auto packet = ReadPacket();
	    if (!packet)
	    {
	        return {};
	    }

	    auto someFrames = mDecoder.DecodePacket(*packet);
	    someFrames = mResampler.Resample(someFrames);

	    if (someFrames.empty())
	    {
	        return {};
	    }
	    else if (someFrames.size() == 1)
	    {
	        return someFrames[0];
	    } else
	    {
	        auto result = std::move(someFrames[0]);
	        mLeftoverFrames.insert(mLeftoverFrames.end(), someFrames.begin() + 1, someFrames.end());
	        return result;
	    }
	}



	Standard::Option<Packet> AudioFile::ReadPacket()
	{
	    Packet packet;

	    // Read packet, ignoring those from other streams
	    int result;
	    do
	    {
	        result = av_read_frame(mFile, *packet);
	        Assert(result == 0 || result == AVERROR_EOF);
	    }
	    while (packet->stream_index != *mStreamIndex && result != AVERROR_EOF);

	    switch (result)
	    {
	        default:
	            return {};

	        case AVERROR_EOF:
	            mIsEof = true;
	            [[fallthrough]];
	        case 0:
	            return packet;
	    }
	}
}