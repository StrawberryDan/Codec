//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Codec
#include "Codec/MediaFile.hpp"
// Core
#include "Strawberry/Core/IO/Error.hpp"
#include "Strawberry/Core/Markers.hpp"
#include "Strawberry/Core/Util/Strings.hpp"


using Strawberry::Core::Assert;


namespace Strawberry::Codec
{
	Core::Result<MediaFile, Error> MediaFile::Open(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path)) return ErrorFileNotFound(path);

		MediaFile file;
		file.mPath  = path;
		auto result = avformat_open_input(&file.mFile, path.string().c_str(), nullptr, nullptr);
		if (result != 0) return ErrorFileNotSupported(path);

		result = avformat_find_stream_info(file.mFile, nullptr);
		if (result != 0) return ErrorFileNotSupported(path);

		return file;
	}


	MediaFile::MediaFile(MediaFile&& other) noexcept
		: mPath(std::exchange(other.mPath, {}))
		, mFile(std::exchange(other.mFile, nullptr))
		, mOpenStreams(std::move(other.mOpenStreams)) {}


	MediaFile& MediaFile::operator=(MediaFile&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return (*this);
	}


	MediaFile::~MediaFile()
	{
		if (mFile)
		{
			avformat_close_input(&mFile);
		}
	}


	MediaStreamInfo MediaFile::GetStreamInfo(size_t index) const
	{
		MediaStreamInfo info{};

		switch (mFile->streams[index]->codecpar->codec_type)
		{
			case AVMEDIA_TYPE_VIDEO:
				info.MediaType = MediaType::Video;
				break;
			case AVMEDIA_TYPE_AUDIO:
				info.MediaType = MediaType::Audio;
				break;
			case AVMEDIA_TYPE_SUBTITLE:
				info.MediaType = MediaType::Subtitle;
				break;
			default:
				info.MediaType = MediaType::Unknown;
				break;
		}

		info.Index           = index;
		info.Stream          = mFile->streams[index];
		info.CodecParameters = info.Stream->codecpar;

		return info;
	}


	MediaStream MediaFile::GetStream(size_t index) const
	{
		Assert(index < mFile->nb_streams);
		return MediaStream(*this, index);
	}


	Core::Optional<MediaStream> MediaFile::GetBestStream(MediaType type) const
	{
		Core::Optional<AVMediaType> streamTypeImpl;
		switch (type)
		{
		case MediaType::Audio:
			streamTypeImpl = AVMEDIA_TYPE_AUDIO;
			break;
		case MediaType::Video:
			streamTypeImpl = AVMEDIA_TYPE_VIDEO;
			break;
		case MediaType::Subtitle:
			streamTypeImpl = AVMEDIA_TYPE_SUBTITLE;
			break;
		case MediaType::Unknown:
		default:
			Core::Unreachable();
		}

		Assert(streamTypeImpl.HasValue());
		auto index = av_find_best_stream(mFile, *streamTypeImpl, -1, -1, nullptr, 0);
		if (index >= 0) return GetStream(index);

		return Core::NullOpt;
	}


	const std::filesystem::path& MediaFile::GetPath() const
	{
		return mPath;
	}
} // namespace Strawberry::Codec
