project(protobuf_any)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-O2 -g -Wall -std=c++0x")

set(CMAKE_BUILD_TYPE MinSizeRel)

add_library(pb2any STATIC pb2any.cpp)  
