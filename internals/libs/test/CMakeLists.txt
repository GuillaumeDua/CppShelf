# Internal test-support target: csl::internal::test
#
# Provides the shared test fixtures (csl::internal::test::...) and pulls Catch2 for the component test suites.
#
# Not a public component:
# - it lives outside libs/,
# - is never installed/exported,
# - and does not go through csl_add_component().

add_library(csl_internal_test INTERFACE)
add_library(csl::internal::test ALIAS csl_internal_test)

target_include_directories(csl_internal_test INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

if (NOT TARGET Catch2::Catch2)
    include(csl/get_cpm)
    csl_get_cpm()
    CPMAddPackage(
        NAME                Catch2
        GITHUB_REPOSITORY   catchorg/Catch2
        VERSION             3.14.0
    )
endif()

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
