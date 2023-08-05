cmake_minimum_required(VERSION 3.00)
project(StrawberryCodec)
set(CMAKE_CXX_STANDARD 23)
find_package(PkgConfig REQUIRED)


pkg_check_modules(FFMPEG REQUIRED
		libavcodec
		libavdevice
		libavfilter
		libavformat
		libavutil
		libswresample
		libswscale)
message(STATUS "Using FFMPEG found at ${FFMPEG_LIBRARY_DIRS}")



find_path(SODIUM_INCLUDE_DIR PATHS sodium.h
	HINTS /usr/local/include)
find_library(SODIUM_LIBRARY REQUIRED
		NAMES libsodium.a)


if(NOT TARGET StrawberryCore AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../Core AND IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Core)
	add_subdirectory(../Core ${CMAKE_CURRENT_BINARY_DIR}/Core)
endif()


add_library(StrawberryCodec STATIC
		include/Codec/Audio/Decoder.hpp
		include/Codec/Audio/Encoder.hpp
		include/Codec/Audio/Frame.hpp
		include/Codec/Audio/FrameFormat.hpp
		include/Codec/Audio/FrameResizer.hpp
		include/Codec/Audio/Mixer.hpp
		include/Codec/Audio/Playlist.hpp
		include/Codec/Audio/Resampler.hpp
		include/Codec/CodecParameters.hpp
		include/Codec/Constants.hpp
		include/Codec/MediaFile.hpp
		include/Codec/MediaStream.hpp
		include/Codec/Muxer.hpp
		include/Codec/Packet.hpp
		include/Codec/SodiumEncrypter.hpp
		src/Audio/Decoder.cpp
		src/Audio/Encoder.cpp
		src/Audio/Frame.cpp
		src/Audio/FrameFormat.cpp
		src/Audio/FrameResizer.cpp
		src/Audio/Mixer.cpp
		src/Audio/Playlist.cpp
		src/Audio/Resampler.cpp
		src/MediaFile.cpp
		src/MediaStream.cpp
		src/Muxer.cpp
		src/Packet.cpp
		src/SodiumEncrypter.cpp
)


target_include_directories(StrawberryCodec PUBLIC include ${FFMPEG_INCLUDE_DIRS} ${SODIUM_INCLUDE_DIR})
target_link_libraries(StrawberryCodec PUBLIC StrawberryCore ${FFMPEG_LIBRARIES} ${SODIUM_LIBRARY})
target_link_directories(StrawberryCodec PUBLIC ${FFMPEG_LIBRARY_DIRS})
# We need this for libavfilter
target_compile_definitions(StrawberryCodec PUBLIC __STDC_CONSTANT_MACROS)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(StrawberryCodec PUBLIC "-framework Security" "-framework VideoToolbox" "-framework AudioToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices" "-framework CoreImage" "-framework CoreGraphics" "-framework VideoToolbox" "-framework OpenGL" "-framework Metal" "-framework Foundation" "-framework AppKit")
endif()


add_executable(StrawberryCodecTest test/Main.cpp)
target_link_libraries(StrawberryCodecTest PRIVATE StrawberryCodec)