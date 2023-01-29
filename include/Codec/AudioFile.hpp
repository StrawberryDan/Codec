#pragma once



#include <string>
#include "Core/Option.hpp"
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

		Core::Option<Frame> ReadFrame();

		[[nodiscard]] inline bool IsEof() const { return mIsEof; }

	private:
		Core::Option<Packet> ReadPacket();

		AVFormatContext* mFile;
		const AVCodec* mCodec;
		Core::Option<int> mStreamIndex;
		bool mIsEof;

		Core::Option<Decoder>   mDecoder;
		std::vector<Frame>      mLeftoverFrames;
	};
}