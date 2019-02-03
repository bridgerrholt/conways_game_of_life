cmake_minimum_required(VERSION 3.8)
project(lodepng)

set(CMAKE_CXX_STANDARD 98)

set(SOURCE_FILES lodepng.cpp lodepng.h)
add_library(lodepng STATIC ${SOURCE_FILES})