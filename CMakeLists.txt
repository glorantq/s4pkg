cmake_minimum_required(VERSION 3.21)
project(s4pkg
        VERSION 0.0.1
        DESCRIPTION "A C++ library for working with Sims 4 package files"
        HOMEPAGE_URL "glorantv.hu"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(GenerateExportHeader)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmt)

file(GLOB_RECURSE LIB_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.h)
add_library(s4pkg SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/library.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/package/impl/inmemorypackage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/package/ipackage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/package/packages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/streams.cpp
    ${LIB_HEADER_FILES})

target_link_libraries(s4pkg PRIVATE fmt::fmt)

generate_export_header(s4pkg
        EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/s4pkg/internal/export.h")

target_include_directories(s4pkg PUBLIC include ${CMAKE_BINARY_DIR})
set_target_properties(s4pkg PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION}
        C_VISIBILITY_PRESET       hidden
        CXX_VISIBILITY_PRESET     hidden
        VISIBILITY_INLINES_HIDDEN YES)

file(GLOB_RECURSE TEST_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp)
add_executable(s4pkg_test ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
    ${TEST_HEADER_FILES})
target_include_directories(s4pkg_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/)
target_link_libraries(s4pkg_test PRIVATE s4pkg)
