function(glance_read_binary_resource input_file output_bytes output_size)
    file(READ "${input_file}" resource_hex HEX)
    string(LENGTH "${resource_hex}" resource_hex_length)
    math(EXPR resource_size "${resource_hex_length} / 2")

    string(REGEX REPLACE "(.{32})" "\\1\n" resource_hex_lines "${resource_hex}")
    string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1, " resource_bytes "${resource_hex_lines}")

    set(${output_bytes} "${resource_bytes}" PARENT_SCOPE)
    set(${output_size} "${resource_size}" PARENT_SCOPE)
endfunction()

function(glance_configure_embedded_resources output_source)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/resources/help.md" GLANCE_HELP_MARKDOWN)
    glance_read_binary_resource(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/highlight/highlight.min.js"
        GLANCE_HIGHLIGHT_JS_BYTES
        GLANCE_HIGHLIGHT_JS_SIZE
    )
    glance_read_binary_resource(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/highlight/github-dark.min.css"
        GLANCE_HIGHLIGHT_CSS_BYTES
        GLANCE_HIGHLIGHT_CSS_SIZE
    )
    glance_read_binary_resource(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/glance-icon.png"
        GLANCE_ICON_PNG_BYTES
        GLANCE_ICON_PNG_SIZE
    )
    glance_read_binary_resource(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/glance-logo.png"
        GLANCE_LOGO_PNG_BYTES
        GLANCE_LOGO_PNG_SIZE
    )

    set(GLANCE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    file(MAKE_DIRECTORY "${GLANCE_GENERATED_DIR}")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/EmbeddedResources.cpp.in"
        "${GLANCE_GENERATED_DIR}/EmbeddedResources.cpp"
        @ONLY
    )

    set(${output_source} "${GLANCE_GENERATED_DIR}/EmbeddedResources.cpp" PARENT_SCOPE)
endfunction()
