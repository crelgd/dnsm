cmake_minimum_required(VERSION 3.5)
project(dnsm)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-utf-8)
elseif (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-Os -utf-8)
    elseif (MSVC)
        add_compile_options(/Os /utf-8)
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)

