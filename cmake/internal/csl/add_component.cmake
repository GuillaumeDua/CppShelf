include_guard(GLOBAL)

include(CMakeParseArguments)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(csl/print_aligned)

### Components - Targets - <project>::<name>
function(csl_add_component)

    set(options)
    set(oneValueArgs NAME PROJECT_NAME)
    set(multiValueArgs)
    cmake_parse_arguments(csl_add_component
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if (DEFINED csl_add_component_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_add_component] error: UNPARSED_ARGUMENTS = [${csl_add_component_UNPARSED_ARGUMENTS}]")
    endif()
    if (DEFINED csl_add_component_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[csl_add_component] error: KEYWORDS_MISSING_VALUES = [${csl_add_component_KEYWORDS_MISSING_VALUES}]")
    endif()
    if (NOT DEFINED csl_add_component_NAME)
        message(FATAL_ERROR "[csl_add_component] error: NAME argument is required")
    endif()

    if (NOT DEFINED csl_add_component_PROJECT_NAME)
        set(csl_add_component_PROJECT_NAME ${PROJECT_NAME})
    endif()

    message(STATUS "[${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME}]")

    set(csl_add_component_NAME          ${csl_add_component_NAME} PARENT_SCOPE)
    set(csl_add_component_PROJECT_NAME  ${csl_add_component_PROJECT_NAME} PARENT_SCOPE)

    add_library(${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME} INTERFACE)
    add_library(${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME} ALIAS ${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME})
    # Export as csl::<name> (not csl::csl_<name>): install(EXPORT NAMESPACE csl::) prepends to EXPORT_NAME.
    set_target_properties(${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME} PROPERTIES
        EXPORT_NAME ${csl_add_component_NAME}
    )

    set(csl_add_component_PATH ${PROJECT_SOURCE_DIR}/libs/${csl_add_component_NAME})

    target_include_directories(${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME} INTERFACE
        $<BUILD_INTERFACE:${csl_add_component_PATH}/includes/${csl_add_component_NAME}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    set(_csl_options_cmake "${csl_add_component_PATH}/cmake/options.cmake")
    if (EXISTS "${_csl_options_cmake}")
        message(VERBOSE "[${csl_add_component_PROJECT_NAME}] ${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME} : loading options from ${_csl_options_cmake}")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include("${_csl_options_cmake}")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    # Interop invariant: a component's interface must NOT link another csl component. Inter-component
    # enhancements are soft (__has_include + fallback in the header), never hard CMake links, so every
    # component exports independently (no inter-component find_dependency, no export ordering). A sibling
    # header is made reachable by the consumer (or the shared install include-root), never by the component.
    get_target_property(_csl_iface_libs
        ${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME} INTERFACE_LINK_LIBRARIES
    )
    if (_csl_iface_libs)
        foreach(_csl_iface_lib IN LISTS _csl_iface_libs)
            # match anywhere: a sibling link is forbidden in any scope, incl. a BUILD_INTERFACE-scoped one
            if (_csl_iface_lib MATCHES "${csl_add_component_PROJECT_NAME}::")
                message(FATAL_ERROR
                    "[${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME}] links sibling component "
                    "[${_csl_iface_lib}] in its interface. Inter-component interops must be soft "
                    "(__has_include + fallback), never a hard link. Remove that target_link_libraries()."
                )
            endif()
        endforeach()
    endif()

    # test
    if (CSL_TEST_${csl_add_component_NAME})
        set(_csl_tests_dir "${csl_add_component_PATH}/tests")
        if (EXISTS "${_csl_tests_dir}/CMakeLists.txt")
            enable_testing()
            message(VERBOSE "[${csl_add_component_PROJECT_NAME}] ${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME} (tests)")
            list(APPEND CMAKE_MESSAGE_INDENT "   ")
            add_subdirectory(${_csl_tests_dir})
            list(POP_BACK CMAKE_MESSAGE_INDENT)
        endif()
    endif()

    # example
    if (CSL_EXAMPLE_${csl_add_component_NAME})
        set(_csl_examples_dir "${csl_add_component_PATH}/examples")
        if (EXISTS "${_csl_examples_dir}/CMakeLists.txt")
            message(VERBOSE "[${csl_add_component_PROJECT_NAME}] ${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME} (example)")
            list(APPEND CMAKE_MESSAGE_INDENT "   ")
            add_subdirectory(${_csl_examples_dir})
            list(POP_BACK CMAKE_MESSAGE_INDENT)
        else()
            message(AUTHOR_WARNING "[${csl_add_component_PROJECT_NAME}] ${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME}: no examples detected, missing ${_csl_examples_dir} ...")
        endif()
    endif()

    # install
    if (CSL_INSTALL)

        set(_csl_cmake_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${csl_add_component_PROJECT_NAME}")

        # The installed csl package is dependency-free:
        #
        # - An exported target must not link an external package.
        # - Optional third-party support (e.g. fmt) are a consumer/source-time opt-ins, never shipped nor declared as a package dependency.
        #
        # Reject early if one leaked into the public interface, rather than emitting a config which would references an unfindable target.
        get_target_property(_csl_link_libs
            ${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME} INTERFACE_LINK_LIBRARIES
        )
        if (_csl_link_libs)
            foreach(_csl_lib IN LISTS _csl_link_libs)
                # BUILD_INTERFACE-scoped links are stripped from the export - they never reach the installed package.
                if (_csl_lib MATCHES "BUILD_INTERFACE") # NOTE: $<BUILD_INTERFACE:…> breaks vscode cmake synthax coloration
                    continue()
                endif()
                if (_csl_lib MATCHES "::" AND NOT _csl_lib MATCHES "^${csl_add_component_PROJECT_NAME}::")
                    message(FATAL_ERROR
                        "[${csl_add_component_PROJECT_NAME}::${csl_add_component_NAME}] links external target "
                        "[${_csl_lib}] and cannot be installed: the csl package is dependency-free. "
                        "Disable the opt-in feature that adds it for install builds, or set CSL_INSTALL=OFF."
                    )
                endif()
            endforeach()
        endif()

        # public headers: install(DIRECTORY) preserves nested layout (e.g. ensure's cxx_17/ + cxx_20/)
        install(DIRECTORY   ${csl_add_component_PATH}/includes/${csl_add_component_NAME}/csl
            DESTINATION     ${CMAKE_INSTALL_INCLUDEDIR}
        )

        install(TARGETS ${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME}
            EXPORT      csl-${csl_add_component_NAME}-targets
        )
        install(EXPORT  csl-${csl_add_component_NAME}-targets
            FILE        csl-${csl_add_component_NAME}-targets.cmake
            NAMESPACE   ${csl_add_component_PROJECT_NAME}::
            DESTINATION ${_csl_cmake_install_dir}
        )

        # per-component config file (minimal: the package is dependency-free)
        set(CSL_COMPONENT_NAME ${csl_add_component_NAME})
        configure_package_config_file(
            "${PROJECT_SOURCE_DIR}/cmake/csl/csl-component-config.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/csl-${csl_add_component_NAME}-config.cmake"
            INSTALL_DESTINATION ${_csl_cmake_install_dir}
        )
        install(FILES   "${CMAKE_CURRENT_BINARY_DIR}/csl-${csl_add_component_NAME}-config.cmake"
            DESTINATION ${_csl_cmake_install_dir}
        )
    endif()

    # doc
    # TODO

    # benchmark
    # TODO

endfunction()
