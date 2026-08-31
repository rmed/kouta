cmake_minimum_required(VERSION 3.18)

project(kouta
    VERSION 1.0.0
    DESCRIPTION "A small framework with reusable elements and components."
    HOMEPAGE_URL "https://github.com/rmed/kouta"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Options
option(KOUTA_BUILD_TESTS "Enable compilation of tests" OFF)

# Third-party module versions required
set(SOCI_MIN_VERSION "4.1")

# Paths
set(_kouta_include_dir "${PROJECT_SOURCE_DIR}/include")
set(_kouta_src_dir "${PROJECT_SOURCE_DIR}/src")
set(_kouta_third_party_dir "${PROJECT_SOURCE_DIR}/third-party")

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
target_include_directories(kouta_callback
    INTERFACE
        ${_kouta_include_dir}
)

# Kouta utils library
add_library(kouta_utils
    SHARED
        "${_kouta_src_dir}/utils/logger-aware.cpp"
)

add_library("kouta::utils" ALIAS kouta_utils)
target_include_directories(kouta_utils
    PUBLIC
        ${_kouta_include_dir}
)

# Kouta async library
#
# Requires:
#
# - asio
add_library(kouta_async
    SHARED
        "${_kouta_src_dir}/async/component.cpp"
        "${_kouta_src_dir}/async/context.cpp"
        "${_kouta_src_dir}/async/root.cpp"
        "${_kouta_src_dir}/async/timer.cpp"
)

add_library("kouta::async" ALIAS kouta_async)

target_include_directories(kouta_async
    PUBLIC
        ${_kouta_include_dir}

    PRIVATE
        "${_kouta_third_party_dir}/asio/include"
)

target_link_libraries(kouta_async PUBLIC kouta_callback)


# Tests
add_subdirectory(tests)
