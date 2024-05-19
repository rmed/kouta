cmake_minimum_required(VERSION 3.27)

project(kouta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Enable compilation of tests" OFF)

# Boost
set(BOOST_MIN_VERSION "1.78.0")

find_package(Boost ${BOOST_MIN_VERSION}
    REQUIRED
        system
)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# Google Test
set(KOUTA_TESTS_ENABLED false)

if(BUILD_TESTS)
    find_package(GTest)

    if(NOT GTEST_FOUND)
        message("GTest was not found. Test target won't be compiled")
    else()
        enable_testing()
        set(KOUTA_TESTS_ENABLED true)
    endif()
endif()

# Kouta
include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory("kouta/base")
add_subdirectory("kouta/io")

# Kouta tests
if(KOUTA_TESTS_ENABLED)
    add_executable(kouta-tests
        # No source is provided, as it is linking against gtest_main
    )

    target_link_libraries(kouta-tests
        PUBLIC
            gtest
            gtest_main
            kouta-base-tests
            kouta-io-tests
    )

    gtest_discover_tests(kouta-tests)
endif()
