set(component_name mp)

function(csl_mp_make_test_target)
    
	set(options)
    set(oneValueArgs   RESULT)
    set(multiValueArgs DEFINITIONS)

    cmake_parse_arguments(arg
        "${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
        ${ARGN}
    )
	if (NOT "${arg_UNPARSED_ARGUMENTS}" STREQUAL "")
		message(FATAL_ERROR "[csl_mp_make_test_target] Unparsed arguments [${arg_UNPARSED_ARGUMENTS}]")
	endif()

	set(target_name_prefix test.${PROJECT_NAME}_${component_name}.compile_time)
	list(JOIN arg_DEFINITIONS "." arg_DEFINITIONS_joined)
	string(REPLACE "=" "-" arg_DEFINITIONS_joined "${arg_DEFINITIONS_joined}")
    set(target_name "${target_name_prefix}.${arg_DEFINITIONS_joined}")

    add_executable(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
    add_dependencies(${target_name} ${PROJECT_NAME}::${component_name})
    target_link_libraries(${target_name}
        PRIVATE
            ${PROJECT_NAME}::${component_name}
    )
    target_include_directories(${target_name}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/includes
    )

	foreach(definition IN LISTS arg_DEFINITIONS)

		string(REGEX MATCH "^[^=]+" key "${definition}")

		target_compile_definitions(${target_name}
			PUBLIC
				${definition}
		)		
	endforeach(definition IN LISTS arg_DEFINITIONS)

    add_test(
        NAME    ${PROJECT_NAME}_test_${component_name}.${arg_DEFINITIONS_joined}
        COMMAND $<TARGET_FILE:${target_name}>
    )

	message(VERBOSE "[csl_mp_make_test_target] Generated ${target_name}")

	if (arg_RESULT)
		set(${arg_RESULT} ${target_name} PARENT_SCOPE)
	endif()

endfunction()

add_custom_target(${PROJECT_NAME}_${component_name}_build_all_tests)

set(CSL_MP_TUPLE__IMPLICIT_CONVERSION_variants 0 1)
foreach(value IN LISTS CSL_MP_TUPLE__IMPLICIT_CONVERSION_variants)

	if ("${value}" STREQUAL "")
		set(DEFINITIONS "")
	else()
		set(DEFINITIONS FORCE_CSL_MP_TUPLE__IMPLICIT_CONVERSION=${value})
	endif()

	csl_mp_make_test_target(
		DEFINITIONS ${DEFINITIONS}
		RESULT      target_name
	)
	add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests
		${target_name}
	)
endforeach()
