set(component_name ag)

# csl::ag's formatting examples optionally enhance themselves with csl::typeinfo (clean type names for
# csl::ag::io::typenamed, e.g. "int" instead of the <typeindex> mangled fallback): self-provision it here,
# the same way ag's tests do, so examples build standalone regardless of whether typeinfo is enabled at
# the top level.
if (NOT TARGET csl::typeinfo)
    add_subdirectory(${PROJECT_SOURCE_DIR}/libs/typeinfo ${CMAKE_CURRENT_BINARY_DIR}/_csl_typeinfo)
endif()

file(GLOB ${component_name}_EXAMPLES_LIST
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

foreach(example_file IN LISTS ${component_name}_EXAMPLES_LIST)

    cmake_path(GET example_file STEM example_name)

    set(target_name example_${component_name}_${example_name})

    # NOTE: csl::typeinfo gives `csl::ag::io::typenamed` clean type names (e.g. "int") instead of the <typeindex> mangled fallback.
    set(example_extra_libs ${PROJECT_NAME}::typeinfo)

    # NOTE: fmtlib support requires linkage with fmt.
    if (example_name MATCHES "fmtlib")

        if (NOT TARGET fmt::fmt-header-only)
            include(csl/get_cpm)
            csl_get_cpm()
            CPMAddPackage(
                NAME              fmt
                GITHUB_REPOSITORY fmtlib/fmt
                GIT_TAG           12.1.0
            )
        endif()

        list(APPEND example_extra_libs fmt::fmt-header-only)
    endif()

    add_executable(${target_name} ${example_file})
    add_dependencies(${target_name} ${PROJECT_NAME}::${component_name})
    target_link_libraries(${target_name}
        PRIVATE
            ${PROJECT_NAME}::${component_name}
            csl_compile_warnings
            ${example_extra_libs}
    )

    add_test(
        NAME    ${PROJECT_NAME}_${target_name}
        COMMAND $<TARGET_FILE:${target_name}>
    )
endforeach()
