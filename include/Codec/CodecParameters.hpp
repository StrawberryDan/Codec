#pragma once


#include "Strawberry/Core/Util/Option.hpp"


namespace Strawberry::Codec
{
	enum class SampleFormat
	{         //|-------------|--------|----------------|
			  //| Layout      | Size   |      Data Type |
		None, //|-------------|--------|----------------|
		U8,   //| Interleaved |  8-bit |   unsigned int |
		U8P,  //| Planar      |  8-bit |   unsigned int |
		S16,  //| Interleaved | 16-bit |     signed int |
		S16P, //| Planar      | 16-bit |     signed int |
		S32,  //| Interleaved | 32-bit |     signed int |
		S32P, //| Planar      | 32-bit |     signed int |
		S64,  //| Interleaved | 64-bit |     signed int |
		S64P, //| Planar      | 64-bit |     signed int |
		FLT,  //| Interleaved | 32-bit | floating point |
		FLTP, //| Planar      | 32-bit | floating point |
		DBL,  //| Interleaved | 64-bit | floating point |
		DBLP, //| Planar      | 64-bit | floating point |
	};        //|---------------------------------------|

	bool SampleFormatIsInterleaved(SampleFormat format);
	bool SampleFormatIsPlanar(SampleFormat format);


	enum class PixelFormat
	{              //==========================================|
				   // Monochrome Formats						|
				   //------------------------------------------|
		MONOWHITE, // 1bpp, 0 = white, 1 = black, msb to lsb	|
		MONOBLACK, // 1bpp, 0 = black, 1 = white, msb to lsb	|
				   //==========================================|
				   // Grey Formats								|
				   //------------------------------------------|
		GRAY8,     // 8bpp, unsigned int						|
		GRAY16LE,  // 16bpp, big endian unsigned int			|
		GRAY16BE,  // 16bpp, little endian unsigned int		|
				  //==========================================|
				  // RGB  formats								|
				  //------------------------------------------|
		RGB24,   // 24bpp									|
		RGB48LE, // 48bpp, little endian unsigned ints		|
		RGB48BE, // 48bpp, big endian unsigned ints			|
				 //==========================================|
				 // RGBA formats								|
				 //------------------------------------------|
		ABGR32,   // 32bpp, unsigned int						|
		ARGB32,   // 32bpp, unsigned int						|
		BGRA32,   // 32bpp, unsigned int						|
		RGBA32,   // 32bpp, unsigned int						|
		RGBA64LE, // 64bpp, little endian unsigned int		|
		RGBA64BE, // 64bpp, big endian unsigned int			|
	};            //==========================================|


	struct CodecParameters {
		Core::Option<int64_t> bitrate;
	};


	struct AudioCodecParameters
		: public CodecParameters {
		Core::Option<int>          sampleRate;
		Core::Option<SampleFormat> sampleFormat;
	};


	struct VideoCodecParameters
		: public CodecParameters {
		Core::Option<int>         width;
		Core::Option<int>         height;
		Core::Option<PixelFormat> pixelFormat;
	};
} // namespace Strawberry::Codec
