cmake_minimum_required(VERSION 3.18)

project(kouta
    VERSION 0.1.0
    DESCRIPTION "A small asynchronous application framework based on Boost/Asio, with components that can be reused accross different projects."
    HOMEPAGE_URL "https://github.com/rmed/kouta"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(cmake/helper.cmake)

# Options
option(KOUTA_BUILD_SHARED "Build a shared library instead of static one" OFF)
option(KOUTA_BUILD_TESTS "Enable compilation of tests" OFF)
option(KOUTA_PREFER_HEADER_ONLY_LIBS "Prefer to use header-only instead of shared external libraries where possible" ON)
option(KOUTA_STANDALONE_ASIO "Use (header-only) standalone Asio instead of Boost.Asio where possible" ON)

# Boost
set(BOOST_MIN_VERSION "1.74.0")

include_directories(${PROJECT_SOURCE_DIR}/include)

# Kouta base module

# Select Asio version to use
configure_file("${PROJECT_SOURCE_DIR}/include/kouta/base/asio.hpp.in" "${PROJECT_SOURCE_DIR}/include/kouta/base/asio.hpp")

if(NOT KOUTA_STANDALONE_ASIO)
    # Use Boost.Asio
    find_package(Boost ${BOOST_MIN_VERSION}
        REQUIRED COMPONENTS
            system
        CONFIG
    )

    include_directories(${Boost_SYSTEM_INCLUDE_DIRS})
    link_directories(${Boost_SYSTEM_LIBRARY_DIRS})

    set(_KOUTA_BASE_ASIO_LIB "")

    # Link against library only if required
    if(NOT KOUTA_PREFER_HEADER_ONLY_LIBS)
        set(_KOUTA_BASE_ASIO_LIB "Boost::system")
    endif()
else()
    # Use standalone Asio (no need to link against anything else)
    #
    # Assumes that Asio headers can be found.
    set(_KOUTA_BASE_ASIO_LIB "")
endif()

kouta_add_library(
    HEADERONLY
    TARGET base
    HEADERS
        "base/callback/base-callback.hpp"
        "base/callback/callback-list.hpp"
        "base/callback/deferred-callback.hpp"
        "base/callback/direct-callback.hpp"
        "base/asio.hpp"
        "base/branch.hpp"
        "base/callback.hpp"
        "base/component.hpp"
        "base/root.hpp"
        "base/timer.hpp"

    SOURCES
        "base.cpp"

    LIBS
        ${_KOUTA_BASE_ASIO_LIB}
)

# Kouta I/O module

# Assumes that Boost.endian headers are available/in path
kouta_add_library(
    HEADERONLY
    TARGET io

    HEADERS
        "io/packer.hpp"
        "io/parser.hpp"

    SOURCES
        "io.cpp"

    INTERNAL
        "base"
)

# Kouta Utils module
kouta_add_library(
    HEADERONLY
    TARGET utils

    HEADERS
        "utils/enum-set.hpp"
        "utils/ilogger.hpp"
        "utils/logger-aware.hpp"

    SOURCES
        "utils.cpp"
)

# Kouta DB module

# Requires SOCI.
find_package(SOCI)

if(SOCI_FOUND)
    # SOCI may be compiled with Boost::date_time support.
    find_package(Boost ${BOOST_MIN_VERSION}
        COMPONENTS
            date_time
    )

    kouta_add_library(
        HEADERONLY
        TARGET db
        HEADERS
            "db/params/sqlite-params.hpp"
            "db/query/condition-group.hpp"
            "db/query/query-builder.hpp"
            "db/query/where-builder.hpp"
            "db/abstract-adapter.hpp"
            "db/client.hpp"
            "db/page.hpp"
            "db/result-code.hpp"
            "db/result.hpp"

        SOURCES
            "db.cpp"

        INTERNAL
            "utils"

        LIBS
            SOCI::soci_core
    )
else()
    message("SOCI was not found. kouta-db module will not be enabled")
endif()

# Tests
add_subdirectory("tests")
