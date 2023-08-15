#include "Codec/Muxer.hpp"


#include "Strawberry/Core/Util/Assert.hpp"
#include "Strawberry/Core/Util/Utilities.hpp"


namespace Strawberry::Codec
{
	using namespace Strawberry::Core;


	Muxer::Muxer(const std::string& file)
		: mAVFormatContext(nullptr)
		, mStreams()
		, mStage(Unopened)
	{
		auto result = avformat_alloc_output_context2(&mAVFormatContext, nullptr, nullptr, file.c_str());
		Assert(result >= 0);

		result = avio_open2(&mAVFormatContext->pb, file.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
		Assert(result >= 0);
		mStage = Opened;
	}


	Muxer::Muxer(Muxer&& other) noexcept
		: mAVFormatContext(Take(other.mAVFormatContext))
		, mStreams(Take(other.mStreams))
		, mStage(Replace(other.mStage, Unopened))
	{}


	Muxer& Muxer::operator=(Muxer&& other) noexcept
	{
		mAVFormatContext = Take(other.mAVFormatContext);
		mStreams         = Take(other.mStreams);
		return (*this);
	}


	Muxer::~Muxer()
	{
		Assert(mStage == TrailerWritten || mStage == Finished);
		avio_close(mAVFormatContext->pb);
		avformat_free_context(mAVFormatContext);
	}


	void Muxer::OpenStream(const AVCodecParameters* codecParameters)
	{
		Assert(mStage == Opened);
		auto encoder = avcodec_find_encoder(codecParameters->codec_id);

		auto stream  = avformat_new_stream(mAVFormatContext, encoder);
		Assert(stream != nullptr);

		auto result = avcodec_parameters_copy(stream->codecpar, codecParameters);
		Assert(result >= 0);

		mStreams.push_back(stream);
	}


	void Muxer::WriteHeader()
	{
		Assert(mStage == Opened);
		auto result = avformat_write_header(mAVFormatContext, nullptr);
		Assert(result >= 0);
		mStage = HeaderWritten;
	}


	void Muxer::WritePacket(Packet& packet)
	{
		Assert(mStage == HeaderWritten || mStage == WritingPackets);
		Assert(mStreams.size() > packet->stream_index);
		auto result = av_write_frame(mAVFormatContext, *packet);
		Assert(result >= 0);
		mStage = WritingPackets;
	}


	void Muxer::WriteTrailer()
	{
		Assert(mStage == WritingPackets);
		auto result = av_write_trailer(mAVFormatContext);
		Assert(result >= 0);
		mStage = TrailerWritten;
	}


	void Muxer::Flush()
	{
		Assert(mStage == TrailerWritten);
		avio_flush(mAVFormatContext->pb);
		avformat_flush(mAVFormatContext);
		mStage = Finished;
	}
}// namespace Strawberry::Codec