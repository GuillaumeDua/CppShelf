cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(csl_consumption_find_package_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Consumes a *pre-installed* csl (see build.sh, which installs csl into a directory and points
# CMAKE_PREFIX_PATH at it). Nothing here references the csl source tree.
find_package(csl REQUIRED COMPONENTS ag ensure functional mp typeinfo wf)

# The aggregate meta-target is available for a component-less find_package(csl).
find_package(csl REQUIRED)
if (NOT TARGET csl::csl)
    message(FATAL_ERROR "[${PROJECT_NAME}] expected aggregate target csl::csl to be defined")
endif()

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/../main.cpp)
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
add_test(NAME csl_consumption_find_package COMMAND ${PROJECT_NAME})
