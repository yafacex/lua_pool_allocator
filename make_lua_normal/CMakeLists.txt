project("lua54")
cmake_minimum_required(VERSION 3.0)
# add_subdirectory("modules/hash")

FILE(GLOB_RECURSE c_files ../src/*.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -Wall -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -Wall -s")

include_directories(
    ../src
)

add_executable(
    lua54
    ${c_files}
)
