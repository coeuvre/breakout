cmake_minimum_required(VERSION 3.3)
project(breakout)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/breakout.c
    src/breakout.h)

add_executable(breakout ${SOURCE_FILES})
target_link_libraries(breakout sdl2)