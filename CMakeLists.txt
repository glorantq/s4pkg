cmake_minimum_required(VERSION 3.21)
project(s4pkg
        VERSION 0.0.1
        DESCRIPTION "A C++ library for working with Sims 4 package files"
        HOMEPAGE_URL "glorantv.hu"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(GenerateExportHeader)

add_library(s4pkg SHARED library.cpp)

generate_export_header(s4pkg
        EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/s4pkg/internal/export.h")

target_include_directories(s4pkg PUBLIC include ${CMAKE_BINARY_DIR})
set_target_properties(s4pkg PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION}
        C_VISIBILITY_PRESET       hidden
        CXX_VISIBILITY_PRESET     hidden
        VISIBILITY_INLINES_HIDDEN YES)

add_executable(s4pkg_test ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)
target_include_directories(s4pkg_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/)
target_link_libraries(s4pkg_test PRIVATE s4pkg)
