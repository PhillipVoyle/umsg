project(umsg)
cmake_minimum_required(VERSION 3.10)


add_library(umsg src/tcp_stream.cpp)
target_include_directories(umsg PUBLIC include/)

add_subdirectory(examples)
