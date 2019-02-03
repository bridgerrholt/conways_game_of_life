cmake_minimum_required(VERSION 3.8)
project(conway_static)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp ../../gifenc/gifenc.c)
add_executable(conway_static ${SOURCE_FILES})

target_include_directories(conway_static PUBLIC ../..)


target_include_directories(conway_static PUBLIC ../../../../_testing/matrix/include ../../gifenc ../../lodepng)
target_link_libraries(conway_static lodepng)
