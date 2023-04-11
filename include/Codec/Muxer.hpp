#pragma once



#include <string>
#include <vector>
#include "Packet.hpp"



extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}



namespace Strawberry::Codec
{
	class Muxer
	{
	public:
		explicit Muxer(const std::string& file);
		Muxer(const Muxer&) = delete;
		Muxer& operator=(const Muxer&) = delete;
		Muxer(Muxer&& other) noexcept ;
		Muxer& operator=(Muxer&& other) noexcept ;
		~Muxer();

		void OpenStream(const AVCodecParameters* codecParameters);
		void WriteHeader();
		void WritePacket(Packet& packet);
		void WriteTrailer();
		void Flush();

	private:

		enum WritingStage
		{
			Unopened,
			Opened,
			HeaderWritten,
			WritingPackets,
			TrailerWritten,
			Finished,
		};

		AVFormatContext* mAVFormatContext;
		std::vector<AVStream*> mStreams;
		WritingStage mStage;
	};
}