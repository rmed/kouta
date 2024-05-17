add_library(kouta-base
    STATIC
        "component.cpp"
        "root.cpp"
)

target_link_libraries(kouta-base
    PUBLIC
        Boost::system
)

add_library(Kouta::Base ALIAS kouta-base)

# Tests
if(KOUTA_TESTS_ENABLED)
    add_library(kouta-base-tests
        OBJECT
            "test/base-test.cpp"
            "test/dummy-component.cpp"
    )

    target_link_libraries(kouta-base-tests
        PUBLIC
            gtest
            gmock
            Kouta::Base
    )
endif()
