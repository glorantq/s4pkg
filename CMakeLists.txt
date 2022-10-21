cmake_minimum_required(VERSION 3.22)
project(s4pkg)

set(CMAKE_CXX_STANDARD 17)

add_library(s4pkg SHARED library.cpp)
