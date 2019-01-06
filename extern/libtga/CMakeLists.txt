cmake_minimum_required(VERSION 3.0)

project(libtga VERSION 1.0.1)

option(BUILD_SHARED "Build libtga as a shared library (default OFF)" OFF)
option(TGA_DEBUG "Enable debug definitions in libtga" OFF)

if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -pedantic -Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(LIBTGA_PROJECT_PATH "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(src)

