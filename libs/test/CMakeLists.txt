csl_add_component(NAME test PROJECT_NAME ${PROJECT_NAME})

if (NOT TARGET Catch2::Catch2)
    include(csl/get_cpm)
    CPMAddPackage(
        NAME                Catch2
        GITHUB_REPOSITORY   catchorg/Catch2
        VERSION             3.14.0
    )
endif()

