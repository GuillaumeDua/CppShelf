set(component_name functional)

set(${component_name}_EXAMPLES_LIST
    overview
)

foreach(example_name IN LISTS ${component_name}_EXAMPLES_LIST)

    set(target_name example_${component_name}_${example_name})

    add_executable(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/${example_name}.cpp)
    add_dependencies(${target_name} ${PROJECT_NAME}::${component_name})
    target_link_libraries(${target_name}
        PRIVATE
            ${PROJECT_NAME}::${component_name}
    )

    add_test(
        NAME 	${PROJECT_NAME}_${target_name}
        COMMAND $<TARGET_FILE:${target_name}>
    )
endforeach()
