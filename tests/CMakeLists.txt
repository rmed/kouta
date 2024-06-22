if(KOUTA_BUILD_TESTS)
    find_package(GTest)

    if(NOT GTEST_FOUND)
        message("GTest was not found. Test target won't be compiled")
    else()
        enable_testing()

        set(_test_sources
            "base/dummy-component.cpp"
            "base/test-base.cpp"
            "io/test-packer.cpp"
            "io/test-parser.cpp"
            "io/test-timer.cpp"
        )

        set(_test_libs
            "kouta-base"
            "kouta-io"
        )
        set(_header_libs _test_libs)
        list(TRANSFORM _header_libs APPEND "-header")

        # Static/Shared library version
        add_executable(kouta-tests
            ${_test_sources}
        )

        target_link_libraries(kouta-tests
            PUBLIC
                gmock
                gtest_main
                gtest
                ${_test_libs}
        )

        gtest_discover_tests(kouta-tests)

        # Header-only version
        add_executable(kouta-tests-header
            ${_test_sources}
        )

        target_link_libraries(kouta-tests-header
            PUBLIC
                gmock
                gtest_main
                gtest
        )

        target_link_libraries(kouta-tests-header
            INTERFACE
                ${_header_libs}
        )

        gtest_discover_tests(kouta-tests-header)
    endif()
endif()
