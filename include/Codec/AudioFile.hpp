#pragma once



#include <string>
#include "Standard/Option.hpp"
#include "Packet.hpp"
#include "Decoder.hpp"
#include "Resampler.hpp"



extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}



namespace Strawberry::Codec
{
	class AudioFile
	{
	public:
	    explicit AudioFile(const std::string& path);
	    AudioFile(const AudioFile& other) = delete;
	    AudioFile& operator=(const AudioFile& other) = delete;
	    AudioFile(AudioFile&& other) noexcept ;
	    AudioFile& operator=(AudioFile&& other) noexcept ;
	    ~AudioFile();

	    inline       AVFormatContext* operator*()        { return mFile; }
	    inline const AVFormatContext* operator*()  const { return mFile; }
	    inline       AVFormatContext* operator->()       { return mFile; }
	    inline const AVFormatContext* operator->() const { return mFile; }

	    Option<Frame> ReadFrame();

	    [[nodiscard]] inline bool IsEof() const { return mIsEof; }

	private:
	    Option<Packet> ReadPacket();

	    AVFormatContext* mFile;
	    const AVCodec* mCodec;
	    Option<int> mStreamIndex;
	    bool mIsEof;

	    Decoder             mDecoder;
	    Resampler           mResampler;
	    std::vector<Frame> mLeftoverFrames;
	};
}