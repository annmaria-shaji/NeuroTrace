cmake_minimum_required(VERSION 3.16)
project(neurotrace_host CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(neurotrace_dsp src/band_power.cpp)
target_include_directories(neurotrace_dsp PUBLIC include ../shared)

add_executable(neurotrace_host src/main.cpp src/simulated_source.cpp src/serial_source.cpp)
target_link_libraries(neurotrace_host PRIVATE neurotrace_dsp)
target_compile_options(neurotrace_host PRIVATE -Wall -Wextra)

enable_testing()
add_executable(test_dsp tests/test_dsp.cpp)
target_link_libraries(test_dsp PRIVATE neurotrace_dsp)
add_test(NAME dsp COMMAND test_dsp)
