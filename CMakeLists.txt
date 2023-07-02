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
		libpostproc
		libswresample
		libswscale)
message(STATUS "Using FFMPEG found at ${FFMPEG_LIBRARY_DIRS}")


find_library(SODIUM_LIBRARY REQUIRED
		NAMES libsodium.a)


if(NOT TARGET StrawberryCore AND EXISTS ../Core AND IS_DIRECTORY ../Core)
	add_subdirectory(../Core ${CMAKE_CURRENT_BINARY_DIR}/Core)
endif()


add_library(StrawberryCodec STATIC
		include/Codec/AudioFile.hpp
		include/Codec/AudioFrameResizer.hpp
		include/Codec/CodecParameters.hpp
		include/Codec/Decoder.hpp
		include/Codec/Frame.hpp
		include/Codec/Muxer.hpp
		include/Codec/Encoder.hpp
		include/Codec/Packet.hpp
		include/Codec/Resampler.hpp
		include/Codec/SodiumEncrypter.hpp
		src/AudioFile.cpp
		src/AudioFrameResizer.cpp
		src/Decoder.cpp
		src/Frame.cpp
		src/Muxer.cpp
		src/Encoder.cpp
		src/Packet.cpp
		src/Resampler.cpp
		src/SodiumEncrypter.cpp
		src/FilterGraph.cpp
		include/Codec/FilterGraph.hpp
		src/Filter.cpp
		include/Codec/Filter.hpp
		include/Codec/Constants.hpp
		src/AudioMixer.cpp
		include/Codec/AudioMixer.hpp)


target_include_directories(StrawberryCodec PUBLIC include ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(StrawberryCodec PUBLIC StrawberryCore ${FFMPEG_LIBRARIES} ${SODIUM_LIBRARY})
target_link_directories(StrawberryCodec PUBLIC ${FFMPEG_LIBRARY_DIRS})
# We need this for libavfilter
target_compile_definitions(StrawberryCodec PUBLIC __STDC_CONSTANT_MACROS)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(StrawberryCodec PUBLIC "-framework Security" "-framework VideoToolbox" "-framework AudioToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices")
endif()


add_executable(StrawberryCodecTest test/Main.cpp)
target_link_libraries(StrawberryCodecTest PRIVATE StrawberryCodec)