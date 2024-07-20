set(component_name ag)

# Test: default (no bitfield support)
set(test_name test_${PROJECT_NAME}_${component_name}_compile_time)

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
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

# --- Tests: opt-ins---
# Test: with bitfield support (CSL_AG__ENABLE_BITFIELDS_SUPPORT)
set(test_name test_${PROJECT_NAME}_${component_name}_bitfield_support_compile_time)

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test_bitfield_support.cpp)
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
	NAME 	${PROJECT_NAME}_test_${component_name}_bitfield_support
	COMMAND $<TARGET_FILE:${test_name}>
)

# Test: with fmtlib support (CSL_AG__ENABLE_FMTLIB_SUPPORT)
set(test_name test_${PROJECT_NAME}_${component_name}_fmtlib_support_runtime)

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test_fmtlib_support.cpp)
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
	NAME 	${PROJECT_NAME}_test_${component_name}_fmtlib_support
	COMMAND $<TARGET_FILE:${test_name}>
)
