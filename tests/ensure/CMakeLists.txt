set(component_name ensure)
set(test_name test_${PROJECT_NAME}_${component_name}_compile_time)

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
)

add_test(
	NAME 	${PROJECT_NAME}_validate_${component_name}
	COMMAND $<TARGET_FILE:${test_name}>
)

# --- C++17 ---

set(test_name test_${PROJECT_NAME}_${component_name}_compile_time_cxx_17)

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
set_target_properties(${test_name} PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
)

add_test(
	NAME 	${PROJECT_NAME}_validate_${component_name}_cxx_17
	COMMAND $<TARGET_FILE:${test_name}>
)