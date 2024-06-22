cmake_minimum_required(VERSION 3.27)

project(kouta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(cmake/helper.cmake)

# Options
option(KOUTA_BUILD_TESTS "Enable compilation of tests" OFF)
option(KOUTA_BUILD_SHARED "Build shared library" OFF)

# Boost
set(BOOST_MIN_VERSION "1.78.0")

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED
    COMPONENTS
        system
)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# Kouta
include_directories(${PROJECT_SOURCE_DIR}/include)

kouta_add_library(
    TARGET base
    HEADERS
        "base/callback/base-callback.hpp"
        "base/callback/callback-list.hpp"
        "base/callback/deferred-callback.hpp"
        "base/callback/direct-callback.hpp"
        "base/branch.hpp"
        "base/callback.hpp"
        "base/component.hpp"
        "base/root.hpp"

    SOURCES
        "base.cpp"

    LIBS
        Boost::system
)

kouta_add_library(
    TARGET io
    HEADERS
        "io/packer.hpp"
        "io/parser.hpp"
        "io/timer.hpp"

    SOURCES
        "io.cpp"

    INTERNAL
        "base"
)

# Tests
add_subdirectory("tests")
