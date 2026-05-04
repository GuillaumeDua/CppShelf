csl_add_component(NAME test PROJECT_NAME ${PROJECT_NAME})

include(get_cpm)

if (NOT TARGET Catch2::Catch2)
    CPMAddPackage(
        NAME                Catch2
        GITHUB_REPOSITORY   catchorg/Catch2
        VERSION             3.14.0
    )
endif()
