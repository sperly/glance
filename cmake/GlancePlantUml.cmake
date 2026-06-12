find_program(GLANCE_PLANTUML_EXECUTABLE NAMES plantuml)

function(glance_configure_plantuml target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Cannot configure PlantUML for unknown target: ${target_name}")
    endif()

    if(GLANCE_PLANTUML_EXECUTABLE)
        file(TO_CMAKE_PATH "${GLANCE_PLANTUML_EXECUTABLE}" plantuml_path)
        target_compile_definitions(${target_name}
            PRIVATE GLANCE_PLANTUML_EXECUTABLE="${plantuml_path}"
        )
        message(STATUS "PlantUML preview support: ${GLANCE_PLANTUML_EXECUTABLE}")
    else()
        message(STATUS "PlantUML executable not found; diagrams will use the preview fallback")
    endif()
endfunction()
