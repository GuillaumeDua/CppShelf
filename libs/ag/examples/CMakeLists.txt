set(component_name ag)

file(GLOB ${component_name}_EXAMPLES_LIST
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

foreach(example_file IN LISTS ${component_name}_EXAMPLES_LIST)

    cmake_path(GET example_file STEM example_name)
    set(target_name example_${component_name}_${example_name})

    add_executable(${target_name} ${example_file})
    add_dependencies(${target_name} ${PROJECT_NAME}::${component_name})
    target_link_libraries(${target_name}
        PRIVATE
            ${PROJECT_NAME}::${component_name}
            csl_compile_warnings
    )

    add_test(
        NAME    ${PROJECT_NAME}_${target_name}
        COMMAND $<TARGET_FILE:${target_name}>
    )
endforeach()
