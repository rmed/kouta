cmake_minimum_required(VERSION 3.18)

project(kouta
    VERSION 1.0.0
    DESCRIPTION "A small framework with reusable elements and components for multiple projects."
    HOMEPAGE_URL "https://github.com/rmed/kouta"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(cmake/helper.cmake)

# Options
option(KOUTA_BUILD_TESTS "Enable compilation of tests" OFF)

# Third-party module versions required
set(SOCI_MIN_VERSION "4.1")

# Paths
set(_kouta_include_dir "${PROJECT_SOURCE_DIR}/include")
set(_kouta_src_dir "${PROJECT_SOURCE_DIR}/src")

include_directories(${_kouta_include_dir})

# Kouta callback library
add_library(kouta_callback
    INTERFACE
        "${_kouta_include_dir}/kouta/callback/abstract-callback.hpp"
        "${_kouta_include_dir}/kouta/callback/callback-list.hpp"
        "${_kouta_include_dir}/kouta/callback/direct-callback.hpp"
        "${_kouta_include_dir}/kouta/callback/null-callback.hpp"
)

add_library("kouta::callback" ALIAS kouta_callback)

# Kouta async library


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
find_package(SOCI ${SOCI_MIN_VERSION})

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
            "db/migration/abstract-version.hpp"
            "db/migration/migration-manager.hpp"
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
            SOCI::Core
    )
else()
    message("SOCI was not found. kouta-db module will not be enabled")
endif()

# Tests
add_subdirectory("tests")
