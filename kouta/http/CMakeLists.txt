find_package(Boost ${BOOST_MIN_VERSION}
    COMPONENTS
        url
)

add_library(kouta-http
    STATIC
        "server/router.cpp"
)

target_link_libraries(kouta-http
    PUBLIC
        Boost::system
        Boost::url
)

add_library(Kouta::Http ALIAS kouta-http)

# Tests
if(KOUTA_TESTS_ENABLED)
    add_library(kouta-http-tests
    OBJECT
        "server/test/router-test.cpp"
    )

    target_link_libraries(kouta-http-tests
    PUBLIC
        gtest
        gmock
        Kouta::Http
    )
endif()
