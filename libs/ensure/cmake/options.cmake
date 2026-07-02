# opt-in: CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT
option(CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable iostream support" OFF)
csl_print_aligned(STATUS CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT)
if (${CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT})
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT)
endif()

# opt-in: CSL_ENSURE__ENABLE_FMT_SUPPORT
option(CSL_ENSURE__ENABLE_FMT_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable fmt support" OFF)
csl_print_aligned(STATUS CSL_ENSURE__ENABLE_FMT_SUPPORT)
if (${CSL_ENSURE__ENABLE_FMT_SUPPORT})

    if (NOT TARGET fmt::fmt-header-only AND NOT TARGET fmt::fmt)
        include(csl/get_cpm)
        csl_get_cpm()
        CPMAddPackage(
            NAME              fmt
            GITHUB_REPOSITORY fmtlib/fmt
            GIT_TAG           12.1.0
        )
    endif()

    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_ENSURE__ENABLE_FMT_SUPPORT)
    if (TARGET fmt::fmt-header-only)
        add_dependencies(csl_${csl_add_component_NAME} fmt::fmt-header-only)
        target_link_libraries(csl_${csl_add_component_NAME} INTERFACE fmt::fmt-header-only)
    elseif(target fmt::fmt)
        add_dependencies(csl_${csl_add_component_NAME} fmt::fmt)
        target_link_libraries(csl_${csl_add_component_NAME} INTERFACE fmt::fmt)
    else()
        message(ERROR "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: ill-formed fmt library")
    endif()
endif()

# opt-in: CSL_ENSURE__ENABLE_STD_FORMAT_SUPPORT
option(CSL_ENSURE__ENABLE_STD_FORMAT_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable std::format support" OFF)
csl_print_aligned(STATUS CSL_ENSURE__ENABLE_STD_FORMAT_SUPPORT)
if (${CSL_ENSURE__ENABLE_STD_FORMAT_SUPPORT})
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_ENSURE__ENABLE_STD_FORMAT_SUPPORT)
endif()
