add_library(kouta-io
    STATIC
        "packer.cpp"
        "parser.cpp"
        "timer.cpp"
)

target_link_libraries(kouta-io
    PUBLIC
        Kouta::Base
)

add_library(Kouta::Io ALIAS kouta-io)

# Tests
if(KOUTA_TESTS_ENABLED)
    add_library(kouta-io-tests
        OBJECT
            "test/packer-test.cpp"
            "test/parser-test.cpp"
            "test/timer-test.cpp"
    )

    target_link_libraries(kouta-io-tests
        PUBLIC
            gtest
            gmock
            Kouta::Io
    )
endif()
