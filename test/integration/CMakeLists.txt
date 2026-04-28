cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(csl_integration_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

include(FetchContent)

set(
    ${PROJECT_NAME}_GIT_REPOSITORY "https://github.com/GuillaumeDua/CppShelf"
    CACHE STRING "[${PROJECT_NAME}] repository URL"
)
set(
    ${PROJECT_NAME}_GIT_TAG "main"
    CACHE STRING "[${PROJECT_NAME}] git tag/branch/SHA to fetch"
)

message(STATUS "[${PROJECT_NAME}] using [${${PROJECT_NAME}_GIT_REPOSITORY}] with [${${PROJECT_NAME}_GIT_TAG}]")

if (NOT TARGET csl::ag)
    set(CSL_BUILD_ALL       ON  CACHE BOOL "" FORCE)
    set(CSL_TEST_ALL        OFF CACHE BOOL "" FORCE)
    set(CSL_EXAMPLE_ALL     OFF CACHE BOOL "" FORCE)
    set(CSL_BENCHMARK_ALL   OFF CACHE BOOL "" FORCE)
    set(CSL_DOC_ALL         OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(csl
        GIT_REPOSITORY  ${${PROJECT_NAME}_GIT_REPOSITORY}
        GIT_TAG         ${${PROJECT_NAME}_GIT_TAG}
    )
    FetchContent_MakeAvailable(csl)
    set(_csl_root ${csl_SOURCE_DIR})
else()
    set(_csl_root ${CMAKE_SOURCE_DIR})
endif()

message("[${PROJECT_NAME}] Importing csl cmake modules as [${CSL_CMAKE_MODULE_PATH}]")
list(APPEND CMAKE_MODULE_PATH ${CSL_CMAKE_MODULE_PATH})
include(csl/make_subdirectory_list)
include(csl/cache_entry_to_int)
include(csl/print_aligned)

csl_make_subdirectory_list(
    DIRECTORY   ${_csl_root}/libs
    RESULT      _csl_components
)
message(STATUS "[${PROJECT_NAME}] fetched csl components: [${_csl_components}]")
csl_print_aligned(STATUS "${CMAKE_BUILD_TYPE} - ${CMAKE_CXX_STANDARD}")

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
