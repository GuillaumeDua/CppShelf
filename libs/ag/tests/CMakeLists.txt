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

if (NOT TARGET csl::typeinfo)
    add_subdirectory(${PROJECT_SOURCE_DIR}/libs/typeinfo ${CMAKE_CURRENT_BINARY_DIR}/_csl_typeinfo)
endif()

# ---------------------------------------------------------------------------
# Helper: create one test target with a given source + opt-in definitions.
#
# Named arguments:
#	NAME        - logical name used in the target name (e.g. "compile_time")
#   SOURCE      - one or more source files
#   DEFINITIONS - list of FORCE_* compile definitions (e.g. FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=1)
#   EXTRA_LIBS  - additional link targets
#   RESULT      - optional variable to receive the generated target name
# ---------------------------------------------------------------------------
function(csl_ag_make_test_target)

    cmake_parse_arguments(arg "" "NAME;RESULT" "SOURCE;DEFINITIONS;EXTRA_LIBS" ${ARGN})

    foreach(lib IN LISTS arg_EXTRA_LIBS)
        if(NOT TARGET ${lib})
            message(FATAL_ERROR "[csl_ag_make_test_target] error: EXTRA_LIBS target '${lib}' does not exist (NAME=${arg_NAME})")
        endif()
    endforeach()

    # Build a readable suffix from each FORCE_CSL_AG__ENABLE_<X>_SUPPORT=<0|1>
    set(suffix_parts "")
    foreach(def IN LISTS arg_DEFINITIONS)
        if(def MATCHES "FORCE_CSL_AG__ENABLE_(.+)_SUPPORT=0")
            string(TOLOWER "${CMAKE_MATCH_1}" part)
            list(APPEND suffix_parts "no_${part}")
        elseif(def MATCHES "FORCE_CSL_AG__ENABLE_(.+)_SUPPORT=1")
            string(TOLOWER "${CMAKE_MATCH_1}" part)
            list(APPEND suffix_parts "${part}")
        elseif(def MATCHES "CSL_AG_TEST__WITH_TYPEINFO=0")
            list(APPEND suffix_parts "no_typeinfo")
        elseif(def MATCHES "CSL_AG_TEST__WITH_TYPEINFO=1")
            list(APPEND suffix_parts "typeinfo")
        endif()
    endforeach()
    list(JOIN suffix_parts "." suffix)

    if (suffix)
        set(target_name "test.${PROJECT_NAME}_${component_name}.${arg_NAME}.${suffix}")
    else()
        set(target_name "test.${PROJECT_NAME}_${component_name}.${arg_NAME}")
    endif()

    add_executable(${target_name} ${arg_SOURCE})
    target_compile_definitions(${target_name} PRIVATE ${arg_DEFINITIONS})
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

    if (arg_EXTRA_LIBS)
        add_dependencies(${target_name} ${arg_EXTRA_LIBS})
    endif()

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

# ---------------------------------------------------------------------------------------------
# Test matrix: foreach bitfield in {0, 1}, foreach inject_csl_typeinfo in {with, without}
#
#   - compile_time.cpp  	    (core static_assert suite, unaffected by inject_csl_typeinfo)
#   - <formatting>_support.cpp  (one formatting feature header each: ostream, fmt, std::format)
#
#   inject_csl_typeinfo covers both `typenamed` code paths - CSL_AG_TEST__WITH_TYPEINFO gates
#   the <csl/ag/formatting/typeinfo.hpp> include in the test sources:
#     - typeinfo:    bridge included + csl::typeinfo linked, compile-time demangled names
#     - no_typeinfo: <typeindex> runtime fallback (deterministic, implementation-defined names)
#
#   Outside the matrix:
#     - formatting_coexistence.cpp:      every feature header in one TU
#     - odr_mixed_{bridge,plain}.cpp:    one binary, two TUs, only one includes the bridge
#     - ostream_iword_cross_tu*.cpp:     one binary, two TUs, iword slot must be program-wide
# ---------------------------------------------------------------------------------------------
add_custom_target(${PROJECT_NAME}_${component_name}_build_all_tests)

foreach(bitfields IN ITEMS 0 1)

    csl_ag_make_test_target(
        NAME        compile_time
        SOURCE      ${CMAKE_CURRENT_SOURCE_DIR}/compile_time.cpp
        DEFINITIONS FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=${bitfields}
        EXTRA_LIBS  ${PROJECT_NAME}::internal::test
        RESULT      target_name
    )
    add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

    foreach(inject_csl_typeinfo IN ITEMS 0 1)

        if (${inject_csl_typeinfo})
            set(_maybe_csl_typeinfo ${PROJECT_NAME}::typeinfo)
        else()
            set(_maybe_csl_typeinfo "")
        endif()

        csl_ag_make_test_target(
            NAME            iostream_support
            SOURCE          ${CMAKE_CURRENT_SOURCE_DIR}/iostream_support.cpp
            DEFINITIONS     FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=${bitfields} CSL_AG_TEST__WITH_TYPEINFO=${inject_csl_typeinfo}
            EXTRA_LIBS      ${_maybe_csl_typeinfo}
            RESULT          target_name
        )
        add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

        csl_ag_make_test_target(
            NAME            fmtlib_support
            SOURCE          ${CMAKE_CURRENT_SOURCE_DIR}/fmtlib_support.cpp
            DEFINITIONS     FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=${bitfields} CSL_AG_TEST__WITH_TYPEINFO=${inject_csl_typeinfo}
            EXTRA_LIBS      fmt::fmt-header-only ${_maybe_csl_typeinfo}
            RESULT          target_name
        )
        add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

        csl_ag_make_test_target(
            NAME            std_format_support
            SOURCE          ${CMAKE_CURRENT_SOURCE_DIR}/std_format_support.cpp
            DEFINITIONS     FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=${bitfields} CSL_AG_TEST__WITH_TYPEINFO=${inject_csl_typeinfo}
            EXTRA_LIBS      ${_maybe_csl_typeinfo}
            RESULT          target_name
        )
        add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})
    endforeach()

endforeach()

# Every formatting feature header in a single TU: backends must coexist (each specializes a different framework's entity)
csl_ag_make_test_target(
    NAME        formatting_coexistence
    SOURCE      ${CMAKE_CURRENT_SOURCE_DIR}/formatting_coexistence.cpp
    DEFINITIONS FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT=0 CSL_AG_TEST__WITH_TYPEINFO=1
    EXTRA_LIBS  fmt::fmt-header-only ${PROJECT_NAME}::typeinfo
    RESULT      target_name
)
add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

# ODR sanity (fmt-style feature-header contract):
# - two TUs, only one includes the typeinfo bridge, each formats different types - must link and behave deterministically
csl_ag_make_test_target(
    NAME        odr_mixed
    SOURCE      ${CMAKE_CURRENT_SOURCE_DIR}/odr_mixed_bridge.cpp ${CMAKE_CURRENT_SOURCE_DIR}/odr_mixed_plain.cpp
    EXTRA_LIBS  ${PROJECT_NAME}::typeinfo
    RESULT      target_name
)
add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})

# iword cross-TU contract: two TUs
# - a manipulator applied in one TU must be seen by a print in the other (details::mode_index() must yield one xalloc slot program-wide)
csl_ag_make_test_target(
    NAME        ostream_iword_cross_tu
    SOURCE      ${CMAKE_CURRENT_SOURCE_DIR}/ostream_iword_cross_tu.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ostream_iword_cross_tu_remote.cpp
    RESULT      target_name
)
add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests ${target_name})
