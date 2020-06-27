project(umsg)
cmake_minimum_required(VERSION 3.16)


add_library(umsg INTERFACE)
target_include_directories(umsg INTERFACE include/)

add_subdirectory(examples)
