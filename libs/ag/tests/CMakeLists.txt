set(component_name ag)

include(Catch)

if (NOT TARGET fmt::fmt-header-only)
    include(csl/get_cpm)
    csl_get_cpm()
    CPMAddPackage(
        NAME              fmt
        GITHUB_REPOSITORY fmtlib/fmt
        GIT_TAG           12.1.0
    )
endif()

# ---------------------------------------------------------------------------
# Helper: create one test target with a given source + opt-in definitions.
#
# Named arguments:
#	NAME        - logical name used in the target name (e.g. "compile_time")
#   SOURCE      - single source file
#   DEFINITIONS - list of FORCE_* compile definitions  (e.g. FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=1)
#   EXTRA_LIBS  - additional link targets  (e.g. fmt::fmt-header-only)
#   RESULT      - optional variable to receive the generated target name
# ---------------------------------------------------------------------------
function(csl_ag_make_test_target)
    cmake_parse_arguments(arg "" "NAME;SOURCE;RESULT" "DEFINITIONS;EXTRA_LIBS" ${ARGN})

    # Build a readable suffix from each FORCE_CSL_AG__ENABLE_<X>_SUPPORT=<0|1>
    set(suffix_parts "")
    foreach(def IN LISTS arg_DEFINITIONS)
        if(def MATCHES "FORCE_CSL_AG__ENABLE_(.+)_SUPPORT=0")
            string(TOLOWER "${CMAKE_MATCH_1}" part)
            list(APPEND suffix_parts "no_${part}")
        elseif(def MATCHES "FORCE_CSL_AG__ENABLE_(.+)_SUPPORT=1")
            string(TOLOWER "${CMAKE_MATCH_1}" part)
            list(APPEND suffix_parts "${part}")
        endif()
    endforeach()
    list(JOIN suffix_parts "." suffix)

    set(target_name "test.${PROJECT_NAME}_${component_name}.${arg_NAME}.${suffix}")

    add_executable(${target_name} ${arg_SOURCE})
    target_compile_definitions(${target_name} PRIVATE ${arg_DEFINITIONS})
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
    target_link_libraries(${target_name}
        PRIVATE
            ${PROJECT_NAME}::${component_name}
            Catch2::Catch2WithMain
            csl_compile_warnings
            ${arg_EXTRA_LIBS}
    )
    catch_discover_tests(${target_name})

    message(VERBOSE "[csl_ag_make_test_target] Generated ${target_name}")

    if(arg_RESULT)
        set(${arg_RESULT} ${target_name} PARENT_SCOPE)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Test matrix: foreach bitfield in {0, 1}
#   - compile_time.cpp  	(core static_assert suite)
#   - fmtlib_support.cpp  	(fmt formatter runtime + static_assert suite)
# ---------------------------------------------------------------------------
add_custom_target(${PROJECT_NAME}_${component_name}_build_all_tests)

foreach(bitfields IN ITEMS 0 1)

    csl_ag_make_test_target(
        NAME        compile_time
        SOURCE      ${CMAKE_CURRENT_SOURCE_DIR}/compile_time.cpp
        DEFINITIONS FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=${bitfields}
        EXTRA_LIBS  ${PROJECT_NAME}::test
        RESULT      target_name
    )
    add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

    csl_ag_make_test_target(
        NAME        fmtlib_support
        SOURCE      ${CMAKE_CURRENT_SOURCE_DIR}/fmtlib_support.cpp
        DEFINITIONS FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=${bitfields}
        EXTRA_LIBS  fmt::fmt-header-only
        RESULT      target_name
    )
    add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

    csl_ag_make_test_target(
        NAME        iostream_support
        SOURCE      ${CMAKE_CURRENT_SOURCE_DIR}/iostream_support.cpp
        DEFINITIONS FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=${bitfields}
        RESULT      target_name
    )
    add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

endforeach()
