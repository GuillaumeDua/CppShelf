set(component_name ag)
set(TEST_LIST "")

# Test: default (no bitfield support)
set(test_name test_${PROJECT_NAME}_${component_name}_compile_time)
list(APPEND TEST_LIST ${test_name})

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/without_bitfield_support.cpp)
target_include_directories(${test_name}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/includes
)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
		${PROJECT_NAME}::test
		csl_compile_warnings
)

add_test(
	NAME 	${PROJECT_NAME}_test_${component_name}
	COMMAND $<TARGET_FILE:${test_name}>
)

# --- Tests: opt-ins---
# Test: with bitfield support (CSL_AG__ENABLE_BITFIELDS_SUPPORT)
set(test_name test_${PROJECT_NAME}_${component_name}_bitfield_support_compile_time)
list(APPEND TEST_LIST ${test_name})

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/with_bitfield_support.cpp)
target_include_directories(${test_name}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/includes
)
add_dependencies(${test_name} ${PROJECT_NAME}::${component_name})
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
		${PROJECT_NAME}::test
		csl_compile_warnings
)

add_test(
	NAME 	${PROJECT_NAME}_test_${component_name}_bitfield_support
	COMMAND $<TARGET_FILE:${test_name}>
)

# Test: with fmtlib support (CSL_AG__ENABLE_FMTLIB_SUPPORT)
set(test_name test_${PROJECT_NAME}_${component_name}_fmtlib_support_runtime)

if (NOT TARGET fmt::fmt-header-only)
    CPMAddPackage(
        NAME              fmt
        GITHUB_REPOSITORY fmtlib/fmt
        GIT_TAG           12.1.0
    )
endif()

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/fmtlib_support.cpp)
target_compile_definitions(${test_name} PRIVATE CSL_AG__ENABLE_FMTLIB_SUPPORT)
target_include_directories(${test_name}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/includes
)
add_dependencies(${test_name}
	${PROJECT_NAME}::${component_name}
	fmt::fmt-header-only
)
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
		fmt::fmt-header-only
		Catch2::Catch2WithMain
		csl_compile_warnings
)

include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")
catch_discover_tests(${test_name})
