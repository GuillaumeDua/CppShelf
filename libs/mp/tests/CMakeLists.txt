set(component_name mp)

function(csl_mp_make_test_target)

	set(options)
    set(oneValueArgs   RESULT)
    set(multiValueArgs DEFINITIONS DISABLED_WARNINGS)

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
            csl_compile_warnings
    )
    target_include_directories(${target_name}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/includes
    )

	list(APPEND CMAKE_MESSAGE_INDENT "  ")
	foreach(definition IN LISTS arg_DEFINITIONS)

		string(REPLACE "=" ";" definition_pair "${definition}")
    	list(GET definition_pair 0 key)
    	list(GET definition_pair 1 value)

        get_property(_strings CACHE "${key}" PROPERTY STRINGS)
        if (_strings)
            list(FIND _strings "${value}" value_as_int)
            if (value_as_int EQUAL -1)
                message(FATAL_ERROR "[csl_mp_make_test_target] invalid value [${value}] for [${key}]. Allowed: [${_strings}]")
            endif()
        else()
            set(value_as_int "${value}")
        endif()

		message(VERBOSE "[csl_mp_make_test_target] Setting compile definition: ${key}=${value} (${value_as_int})")

		target_compile_definitions(${target_name}
			PUBLIC
				FORCE_${key}=${value_as_int} # handled on the C++ side, overrides ${key}
		)
	endforeach()

    if (arg_DISABLED_WARNINGS)
        target_compile_options(${target_name} PRIVATE ${arg_DISABLED_WARNINGS})
    endif()
	list(POP_BACK CMAKE_MESSAGE_INDENT)

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

get_property(CSL_MP_TUPLE__IMPLICIT_CONVERSION_allowed_values CACHE CSL_MP_TUPLE__IMPLICIT_CONVERSION PROPERTY STRINGS)
foreach(value IN LISTS CSL_MP_TUPLE__IMPLICIT_CONVERSION_allowed_values)

	set(DEFINITIONS CSL_MP_TUPLE__IMPLICIT_CONVERSION=${value})

	csl_mp_make_test_target(
		DEFINITIONS       ${DEFINITIONS}
		DISABLED_WARNINGS ${CSL_MP_TUPLE__IMPLICIT_CONVERSION_${value}_test_disabled_warnings}
		RESULT            target_name
	)
	add_dependencies(${PROJECT_NAME}_${component_name}_build_all_tests
		${target_name}
	)
endforeach()
