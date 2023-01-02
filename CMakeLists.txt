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



find_library(SODIUM_LIBRARY REQUIRED
        NAMES libsodium.a)



add_library(Codec STATIC
        include/Codec/AudioFile.hpp
        include/Codec/Decoder.hpp
        include/Codec/Frame.hpp
        include/Codec/Muxer.hpp
        include/Codec/OpusEncoder.hpp
        include/Codec/Packet.hpp
        include/Codec/Resampler.hpp
        include/Codec/Sample.hpp
        include/Codec/SodiumEncrypter.hpp
        src/AudioFile.cpp
        src/Decoder.cpp
        src/Frame.cpp
        src/Muxer.cpp
        src/OpusEncoder.cpp
        src/Packet.cpp
        src/Resampler.cpp
        src/Sample.cpp
        src/SodiumEncrypter.cpp
        )

target_include_directories(Codec PUBLIC include ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(Codec PUBLIC Standard ${FFMPEG_LIBRARIES} ${SODIUM_LIBRARY})
target_link_directories(Codec PUBLIC ${FFMPEG_LIBRARY_DIRS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(Codec PUBLIC "-framework Security" "-framework VideoToolbox" "-framework AudioToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices")
endif()

add_executable(Codec_Test test/Main.cpp)
target_include_directories(Codec_Test PRIVATE Codec)
target_link_libraries(Codec_Test PRIVATE Codec)