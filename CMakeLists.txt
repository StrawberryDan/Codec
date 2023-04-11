cmake_minimum_required(VERSION 3.00)
project(Strawberry_Codec)
set(CMAKE_CXX_STANDARD 20)
find_package(PkgConfig REQUIRED)

add_compile_options(-fsanitize=address)
add_link_options   (-fsanitize=address)

pkg_check_modules(FFMPEG REQUIRED
        libavcodec
        libavdevice
        libavfilter
        libavformat
        libavutil
        libpostproc
        libswresample
        libswscale)



find_library(SODIUM_LIBRARY REQUIRED
        NAMES libsodium.a)



set(STRAWBERRY_CORE_SRC_DIR "" CACHE PATH "Set to the location of Strawberry Core source code to use instead of normal searching")
if(NOT ${STRAWBERRY_CORE_SRC_DIR} STREQUAL "")
    add_subdirectory(${STRAWBERRY_CORE_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/Strawberry_Core)
endif()



add_library(Strawberry_Codec STATIC
        include/Codec/AudioFile.hpp
        include/Codec/AudioFrameResizer.hpp
        include/Codec/CodecParameters.hpp
        include/Codec/Decoder.hpp
        include/Codec/Frame.hpp
        include/Codec/Muxer.hpp
        include/Codec/OpusEncoder.hpp
        include/Codec/Packet.hpp
        include/Codec/Resampler.hpp
        include/Codec/SodiumEncrypter.hpp
        src/AudioFile.cpp
        src/AudioFrameResizer.cpp
        src/Decoder.cpp
        src/Frame.cpp
        src/Muxer.cpp
        src/OpusEncoder.cpp
        src/Packet.cpp
        src/Resampler.cpp
        src/SodiumEncrypter.cpp)

target_include_directories(Strawberry_Codec PUBLIC include ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(Strawberry_Codec PUBLIC Strawberry_Core ${FFMPEG_LIBRARIES} ${SODIUM_LIBRARY})
target_link_directories(Strawberry_Codec PUBLIC ${FFMPEG_LIBRARY_DIRS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(Strawberry_Codec PUBLIC "-framework Security" "-framework VideoToolbox" "-framework AudioToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices")
endif()



add_executable(Strawberry_Codec_Test test/Main.cpp)
target_link_libraries(Strawberry_Codec_Test PRIVATE Strawberry_Codec)