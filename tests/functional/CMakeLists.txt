set(component_name functional)

# --- C++20 ---

set(test_name test_${PROJECT_NAME}_${component_name}_cxx20_compile_time)
add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
)

add_test(
	NAME 	${PROJECT_NAME}_test_${component_name}_cxx20
	COMMAND $<TARGET_FILE:${test_name}>
)

# --- C++17 ---

unset(test_name)
set(test_name2 test_${PROJECT_NAME}_${component_name}_cxx17_compile_time)
add_executable(${test_name2} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
set_target_properties(${test_name2} PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
)
add_dependencies(${test_name2} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name2}
	PRIVATE
		${PROJECT_NAME}::${component_name}
)

add_test(
	NAME 	${PROJECT_NAME}_test_${component_name}_cxx17
	COMMAND $<TARGET_FILE:${test_name2}>
)
