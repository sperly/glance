function(glance_add_format_targets)
    find_program(CLANG_FORMAT_EXE NAMES clang-format)
    if(CLANG_FORMAT_EXE)
        add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${ARGN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Formatting C++ sources with clang-format"
            VERBATIM
        )

        add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ARGN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking C++ formatting with clang-format"
            VERBATIM
        )
    else()
        add_custom_target(format
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Install clang-format to use the format target."
            VERBATIM
        )

        add_custom_target(format-check
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Install clang-format to use the format-check target."
            COMMAND ${CMAKE_COMMAND} -E false
            VERBATIM
        )
    endif()
endfunction()
