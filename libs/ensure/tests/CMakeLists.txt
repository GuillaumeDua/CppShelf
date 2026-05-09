set(component_name ensure)

include(Catch)

# --- C++20 ---
set(test_name test_${PROJECT_NAME}_${component_name}_cxx20_compile_time)
add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
		${PROJECT_NAME}::test
		Catch2::Catch2WithMain
		csl_compile_warnings
)

catch_discover_tests(${test_name})

# --- C++17 ---
set(test_name test_${PROJECT_NAME}_${component_name}_cxx17_compile_time)
add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
set_target_properties(${test_name} PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
		${PROJECT_NAME}::test
		Catch2::Catch2WithMain
		csl_compile_warnings
)

catch_discover_tests(${test_name})

# Tests: all
add_custom_target(${PROJECT_NAME}_${component_name}_build_all_tests
	DEPENDS ${test_name}
)
