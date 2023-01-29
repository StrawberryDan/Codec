#include "Codec/AudioFile.hpp"
#include "Core/Utilities.hpp"



#include <iostream>



using Strawberry::Core::Assert;
using Strawberry::Core::Take;
using Strawberry::Core::Replace;



namespace Strawberry::Codec
{
	AudioFile::AudioFile(const std::string& path)
		: mFile(nullptr)
		, mCodec(nullptr)
		, mStreamIndex{}
		, mIsEof(false)
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

		result = avformat_seek_file(
				mFile, *mStreamIndex,
				0, 0, std::numeric_limits<int64_t>::max(), 0);
		Assert(result >= 0);
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



	Core::Option<Packet> AudioFile::ReadPacket()
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



	const AVCodec* AudioFile::GetCodec() const
	{
		auto codecID = mFile->streams[*mStreamIndex]->codecpar->codec_id;
		return avcodec_find_decoder(codecID);
	}



	const AVCodecParameters* AudioFile::GetCodecParameters() const
	{
		return mFile->streams[*mStreamIndex]->codecpar;
	}
}