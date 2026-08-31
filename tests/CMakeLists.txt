if(KOUTA_BUILD_TESTS)
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)

    if(NOT GTEST_FOUND)
        message("GTest was not found. Test target won't be compiled")
    else()
        enable_testing()

        if(NOT MSVC)
            set(_pthread pthread)
        endif()


        set(_test_libs
            "kouta_async"
        )

        add_executable(kouta-tests
            "async/dummy-component.cpp"
            "async/test-async.cpp"
            "async/test-timer.cpp"
            "utils/test-enum-set.cpp"
            "utils/test-logger-aware.cpp"
        )

        target_link_libraries(kouta-tests
            PUBLIC
                gmock
                gtest_main
                gtest
                ${_pthread}
                kouta_async
                kouta_utils
        )

        gtest_discover_tests(kouta-tests)
    endif()
endif()
