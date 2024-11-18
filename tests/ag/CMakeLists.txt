set(component_name ag)

# Test: default (no bitfield support)
set(test_name test_${PROJECT_NAME}_${component_name}_compile_time)

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/without_bitfield_support.cpp)
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

add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/with_bitfield_support.cpp)
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

# 3rd-parties
set(FETCHCONTENT_QUIET ON)
include(FetchContent)

# fmt
if (NOT TARGET fmt::fmt-header-only)
	list(APPEND CMAKE_MESSAGE_INDENT "  ")
	FetchContent_Declare(fmt
		GIT_REPOSITORY https://github.com/fmtlib/fmt.git
		GIT_TAG master
	)
	FetchContent_MakeAvailable(fmt)
	list(POP_BACK CMAKE_MESSAGE_INDENT)
endif()

# gtest
if (NOT TARGET GTest::gtest_main)
	list(APPEND CMAKE_MESSAGE_INDENT "  ")
	FetchContent_Declare(googletest
		GIT_REPOSITORY 	https://github.com/google/googletest.git
		GIT_TAG 		v1.15.2
	)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)
	list(POP_BACK CMAKE_MESSAGE_INDENT)
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
	GTest::gtest_main
)
target_link_libraries(${test_name}
	PRIVATE
		${PROJECT_NAME}::${component_name}
		fmt::fmt-header-only
		GTest::gtest_main
)

# add_test(
# 	NAME 	${PROJECT_NAME}_test_${component_name}_fmtlib_support
# 	COMMAND $<TARGET_FILE:${test_name}>
# )
include(GoogleTest)
gtest_discover_tests(${test_name})
