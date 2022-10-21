cmake_minimum_required(VERSION 3.21)
project(s4pkg
        VERSION 0.0.1
        DESCRIPTION "A C++ library for working with Sims 4 package files"
        HOMEPAGE_URL "glorantv.hu"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(s4pkg SHARED library.cpp)
target_include_directories(s4pkg PUBLIC include)