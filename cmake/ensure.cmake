# opt-in: CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT
option(CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT "[${CMAKE_PROJECT_NAME}] csl::${component_name}: enable iostream support" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT set to [${CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT}]")
if (${CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT})
    target_compile_definitions(csl_${component_name}_lib INTERFACE CSL_ENSURE__ENABLE_IOSTREAM_SUPPORT)
endif()

# opt-in: CSL_ENSURE__ENABLE_FMT_SUPPORT
option(CSL_ENSURE__ENABLE_FMT_SUPPORT "[${CMAKE_PROJECT_NAME}] csl::${component_name}: enable fmt support" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_ENSURE__ENABLE_FMT_SUPPORT set to [${CSL_ENSURE__ENABLE_FMT_SUPPORT}]")
if (${CSL_ENSURE__ENABLE_FMT_SUPPORT})
    # 3rd party: fmt
    set(FETCHCONTENT_QUIET ON)
    message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} fetching [fmt] library ...")
   
    if (NOT TARGET fmt::fmt-header-only)
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include(FetchContent)
        FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(fmt)
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    target_compile_definitions(csl_${component_name}_lib INTERFACE CSL_ENSURE__ENABLE_FMT_SUPPORT)
    if (TARGET fmt::fmt-header-only)
        add_dependencies(csl_${component_name}_lib fmt::fmt-header-only)
        target_link_libraries(csl_${component_name}_lib INTERFACE fmt::fmt-header-only)
    elseif(target fmt::fmt)
        add_dependencies(csl_${component_name}_lib fmt::fmt)
        target_link_libraries(csl_${component_name}_lib INTERFACE fmt::fmt)
    else()
        message(ERROR "[${CMAKE_PROJECT_NAME}] csl::${component_name}: ill-formed fmt library")
    endif()
endif()
