# Add library targets for static/shared and header-only libraries
#
# This will generate the following targets:
#
# - kouta-<targetname>: static/shared version
# - kouta::<targetname>: alias for the static/shared version
# - kouta-<targetname>-header: header-only version
# - kouta::<targetname>Header: alias for the header-only version
#
# Args:
#
# - TARGET: name of the target to generate, treated as a suffix
# - HEADERS: list of headers relative to the include/kouta/ directory
# - SOURCES: list of sources relative to the src/ directory
# - INTERNAL: list of internal components to link against
# - LIBS: list of external libraries to link against
#
# Example:
#
# kouta_add_library(
#     TARGET io
#
#     HEADERS
#         io/packer.hpp
#         io/parser.hpp
#         io/timer.hpp
#
#     SOURCES
#         io.cpp
#
#     INTERNAL
#         base
# )
function(kouta_add_library)
    cmake_parse_arguments(ARGS "" "TARGET" "HEADERS;SOURCES;INTERNAL;LIBS" ${ARGN})

    set(_lib_target "kouta-${ARGS_TARGET}")
    set(_header_target "${_lib_target}-header")

    list(TRANSFORM ARGS_HEADERS PREPEND "include/kouta/")
    list(TRANSFORM ARGS_SOURCES PREPEND "src/")

    # Static/Shared library
    if(BUILD_SHARED_LIBS OR KOUTA_BUILD_SHARED)
        add_library(${_lib_target}
            SHARED
                ${ARGS_SOURCES}
                ${ARGS_HEADERS}
        )
    else()
        add_library(${_lib_target}
            STATIC
                ${ARGS_SOURCES}
                ${ARGS_HEADERS}
        )
    endif()

    target_include_directories(${_lib_target}
        INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    )

    # Internal links
    if(ARGS_INTERNAL)
        target_link_libraries(${_lib_target} PUBLIC "kouta-${ARGS_INTERNAL}")
    endif()

    # External libs
    if(ARGS_LIBS)
        target_link_libraries(${_lib_target} PUBLIC ${ARGS_LIBS})
    endif()

    add_library("kouta::${ARGS_TARGET}" ALIAS ${_lib_target})

    # Header-only library
    add_library(${_header_target}
        INTERFACE
            ${ARGS_HEADERS}
    )

    target_include_directories(${_header_target}
        INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    )

    if(ARGS_INTERNAL)
        target_link_libraries(${_header_target} INTERFACE "kouta-${ARGS_INTERNAL}-header")
    endif()

    # External libs
    if(ARGS_LIBS)
        target_link_libraries(${_header_target} INTERFACE ${ARGS_LIBS})
    endif()

    add_library("kouta::${ARGS_TARGET}Header" ALIAS ${_header_target})
endfunction()
