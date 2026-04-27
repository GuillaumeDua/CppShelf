# opt-in: CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT
option(CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable iostream support" OFF)
print_aligned(STATUS CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT)
if (${CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT})
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT)
endif()

# opt-in: CSL_ENSURE__ENABLE_FMT_SUPPORT
option(CSL_ENSURE__ENABLE_FMT_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable fmt support" OFF)
print_aligned(STATUS CSL_ENSURE__ENABLE_FMT_SUPPORT)
if (${CSL_ENSURE__ENABLE_FMT_SUPPORT})

    # REFACTO: use CPM
    # 3rd party: fmt
    set(FETCHCONTENT_QUIET ON)
    message(STATUS "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}] fetching [fmt] library ...")
   
    if (NOT TARGET fmt::fmt-header-only)
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include(FetchContent)
        FetchContent_Declare(fmt
            GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
            GIT_TAG         main
        )
        FetchContent_MakeAvailable(fmt)
        list(POP_BACK CMAKE_MESSAGE_INDENT)
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
