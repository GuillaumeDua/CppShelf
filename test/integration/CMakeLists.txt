cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(csl_integration_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(
    ${PROJECT_NAME}_GIT_REPOSITORY "https://github.com/GuillaumeDua/CppShelf"
    CACHE STRING "[${PROJECT_NAME}] repository URL"
)
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE     _csl_default_git_tag
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_result
)
if (NOT _git_result EQUAL 0 OR _csl_default_git_tag STREQUAL "HEAD")
    set(_csl_default_git_tag "main")
endif()

set(
    ${PROJECT_NAME}_GIT_TAG "${_csl_default_git_tag}"
    CACHE STRING "[${PROJECT_NAME}] git tag/branch/SHA to fetch"
)

macro(import_csl)

    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    if (NOT TARGET csl)
        message(STATUS "[${PROJECT_NAME}] using GIT_REPOSITORY=[${${PROJECT_NAME}_GIT_REPOSITORY}] with GIT_TAG=[${${PROJECT_NAME}_GIT_TAG}] ...")

        set(CSL_BUILD_ALL       ON  CACHE BOOL "" FORCE)
        set(CSL_TEST_ALL        OFF CACHE BOOL "" FORCE)
        set(CSL_EXAMPLE_ALL     OFF CACHE BOOL "" FORCE)
        set(CSL_BENCHMARK_ALL   OFF CACHE BOOL "" FORCE)
        set(CSL_DOC_ALL         OFF CACHE BOOL "" FORCE)

        include(FetchContent)
        FetchContent_Declare(csl
            GIT_REPOSITORY  ${${PROJECT_NAME}_GIT_REPOSITORY}
            GIT_TAG         ${${PROJECT_NAME}_GIT_TAG}
        )
        FetchContent_MakeAvailable(csl)

        if (NOT csl_SOURCE_DIR OR "${csl_SOURCE_DIR}" STREQUAL "")
            message(FATAL_ERROR "[${PROJECT_NAME}] import_csl: unexpected csl_SOURCE_DIR=[${csl_SOURCE_DIR}]")
        endif()

        message(STATUS "[${PROJECT_NAME}] Importing [csl] cmake modules from [${CSL_CMAKE_MODULE_PATH}]")
        list(APPEND CMAKE_MODULE_PATH ${CSL_CMAKE_MODULE_PATH})
    else()
        message(STATUS "[${PROJECT_NAME}] [csl] target already exists.")
    endif()

    list(POP_BACK CMAKE_MESSAGE_INDENT)
endmacro()
import_csl()

include(csl/make_subdirectory_list)
include(csl/cache_entry_to_int)
include(csl/print_aligned)

csl_make_subdirectory_list(
    DIRECTORY   ${csl_SOURCE_DIR}/libs
    RESULT      _csl_components
)
message(STATUS "[${PROJECT_NAME}] fetched csl components: [${_csl_components}]")
csl_print_aligned(STATUS CMAKE_BUILD_TYPE)
csl_print_aligned(STATUS CMAKE_CXX_STANDARD)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        csl::ag
        csl::ensure
        csl::functional
        csl::mp
        csl::typeinfo
        csl::wf
)

enable_testing()
add_test(NAME csl_integration COMMAND ${PROJECT_NAME})
