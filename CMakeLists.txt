cmake_minimum_required(VERSION 3.8)
project(conways_game_of_life)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -O3)

add_subdirectory(src/lodepng)

set(SOURCE_FILES src/main.cpp src/engine.h src/map_engine.h src/digit_count.h src/write_gif.h)
add_executable(conways_game_of_life ${SOURCE_FILES})

add_executable(image_start src/image_start.cpp)

target_include_directories(conways_game_of_life PUBLIC ../_testing/matrix/include src/lodepng)
target_link_libraries(conways_game_of_life lodepng)

target_include_directories(image_start PUBLIC ../_testing/matrix/include src/lodepng)
target_link_libraries(image_start lodepng)

add_subdirectory(src/projects/static)