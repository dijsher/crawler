cmake_minimum_required(VERSION 3.22)
project(crawler)

set(CMAKE_CXX_STANDARD 17)

add_executable(crawler main.cpp)

target_link_libraries(crawler PRIVATE winhttp)
