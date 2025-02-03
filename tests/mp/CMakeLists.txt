set(component_name mp)

set(test_name test_${PROJECT_NAME}_${component_name}_compile_time)

# TODO: duplicate tuple tests with CSL_MP_TUPLE__DISABLE_IMPLICIT_CONVERSION

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
)

target_include_directories(${test_name}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/includes
)

add_test(
	NAME 	${PROJECT_NAME}_validate_${component_name}
	COMMAND $<TARGET_FILE:${test_name}>
)

# Tests: all
add_custom_target("${component_name}_build_all_tests"
	DEPENDS ${test_name}
)
