cmake_minimum_required(VERSION 3.15)

project(MCK_HELPER VERSION 0.0.1)

add_library(MckHelper)

find_library(SNDFILE sndfile)
find_library(SAMPLERATE samplerate)
find_library(JACK NAMES jack PATHS /usr/lib /usr/lib64 /usr/lib64/pipewire-0.3/jack)


add_subdirectory(deps/json)

target_include_directories(MckHelper
    PRIVATE
    ./include
)

target_sources(MckHelper
    PRIVATE
    src/DspHelper.cpp
    src/JackHelper.cpp
    src/Metronome.cpp
    src/Transport.cpp
    src/WaveHelper.cpp
)

target_link_libraries(MckHelper
    PRIVATE
    nlohmann_json
    sndfile
    samplerate
    ${JACK}
)