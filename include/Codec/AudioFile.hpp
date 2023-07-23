#pragma once



#include <string>
#include "Strawberry/Core/Option.hpp"
#include "Strawberry/Core/IO/Producer.hpp"
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
			: Core::IO::Producer<Packet>
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

		virtual Core::Option<Packet> Receive() override;

		[[nodiscard]] inline bool IsEof() const { return mIsEof; }

		[[nodiscard]] const AVCodec*           GetCodec() const;
		[[nodiscard]] const AVCodecParameters* GetCodecParameters() const;

	private:
		AVFormatContext* mFile;
		const AVCodec* mCodec;
		Core::Option<int> mStreamIndex;
		bool mIsEof;
	};
}