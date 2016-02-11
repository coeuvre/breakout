cmake_minimum_required(VERSION 3.3)
project(breakout)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")

set(SOURCE_FILES src/breakout.c)

add_executable(breakout ${SOURCE_FILES})
target_link_libraries(breakout sdl2)
