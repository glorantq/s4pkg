cmake_minimum_required(VERSION 3.21)
project(s4pkg
        VERSION 0.0.2
        DESCRIPTION "A C++ library for working with Sims 4 package files"
        HOMEPAGE_URL "https://glorantq.github.io/TravisStudio/"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(GenerateExportHeader)

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/miniz)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/libjpeg-turbo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/libsquish)

# Let's not build in the source tree okay? (looking at you, miniz)

set_target_properties(miniz PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties(fmt PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties(jpeg PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties(squish PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    WINDOWS_EXPORT_ALL_SYMBOLS YES
    C_VISIBILITY_PRESET       visible
    CXX_VISIBILITY_PRESET     visible
    VISIBILITY_INLINES_HIDDEN NO)

file(GLOB_RECURSE LIB_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.h)
add_library(s4pkg SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/package/impl/inmemorypackage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/package/ipackage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/package/packages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/package/types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/streams.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/globals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/fallbackresourcefactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/fallbackresource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/stb_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/imagecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/dds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/imageresource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ts4/thumbnailresource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ts4/thumbnailresourcefactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ts4/dstresource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ts4/dstresourcefactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ts4/rleresource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ts4/rleresourcefactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/rle.cpp
    ${LIB_HEADER_FILES})

target_link_libraries(s4pkg PRIVATE fmt::fmt miniz jpeg squish)

generate_export_header(s4pkg
        EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/s4pkg/internal/export.h")

target_include_directories(s4pkg PUBLIC include ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(s4pkg PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libjpeg-turbo"
    "${CMAKE_CURRENT_BINARY_DIR}/vendor/libjpeg-turbo"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libsquish")

set_target_properties(s4pkg PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION}
        C_VISIBILITY_PRESET       hidden
        CXX_VISIBILITY_PRESET     hidden
        VISIBILITY_INLINES_HIDDEN YES)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/s4pkg/version.h")


file(GLOB_RECURSE TEST_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp)
add_executable(s4pkg_test ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
    ${TEST_HEADER_FILES})
target_include_directories(s4pkg_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/)
target_link_libraries(s4pkg_test PRIVATE s4pkg)
