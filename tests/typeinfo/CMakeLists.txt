set(component_name typeinfo)
set(test_name test_${PROJECT_NAME}_${component_name}_compile_time)

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 23)
target_include_directories(${test_name}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/includes
)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
)

add_test(
	NAME 	${PROJECT_NAME}_test_${component_name}
	COMMAND $<TARGET_FILE:${test_name}>
)