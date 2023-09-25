cmake_minimum_required(VERSION 3.22)

project(uring_server)

set(CMAKE_CXX_STANDARD 11)

add_executable(program main.cpp server.cpp)

target_include_directories(program PRIVATE ./liburing/src/include)
target_link_directories(program PRIVATE ./liburing/src/)

target_compile_options(program PRIVATE -Wall -O2 -D_GNU_SOURCE -luring)
target_link_libraries(program PRIVATE uring)
