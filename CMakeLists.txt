cmake_minimum_required(VERSION 3.0)
project(mck-helper VERSION 0.0.1 LANGUAGES CXX)

find_package(doctest REQUIRED)

add_executable(tests ./test/config.cpp ./test/conversion.cpp ./src/DspHelper.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_link_libraries(tests PRIVATE doctest::doctest)