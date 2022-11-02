add_library(Codec STATIC
        include/Codec/AudioFile.hpp
        src/AudioFile.cpp
        src/Packet.cpp
        include/Codec/Packet.hpp
        src/Decoder.cpp
        include/Codec/Decoder.hpp
        src/Frame.cpp
        include/Codec/Frame.hpp
        src/Resampler.cpp
        include/Codec/Resampler.hpp
        src/OpusEncoder.cpp
        include/Codec/OpusEncoder.hpp
        include/Codec/Sample.hpp
        src/Sample.cpp
        src/Muxer.cpp
        include/Codec/Muxer.hpp
        src/SodiumEncrypter.cpp
        include/Codec/SodiumEncrypter.hpp)
target_include_directories(Codec PUBLIC include)
target_link_libraries(Codec PUBLIC FFMPEG sodium Standard)

add_executable(Codec_Test test/Main.cpp)
target_include_directories(Codec_Test PRIVATE Codec)
target_link_libraries(Codec_Test PRIVATE Codec)