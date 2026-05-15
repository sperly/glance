function(glance_add_core_tests)
    if(NOT BUILD_TESTING)
        return()
    endif()

    add_executable(glance_core_tests
        tests/core_tests.cpp
        src/Document.cpp
        src/Document.h
        src/DocumentManager.cpp
        src/DocumentManager.h
        src/MarkdownFlavor.cpp
        src/MarkdownFlavor.h
        src/MarkdownRenderer.cpp
        src/MarkdownRenderer.h
        src/MarkdownValidator.cpp
        src/MarkdownValidator.h
    )

    target_link_libraries(glance_core_tests ${wxWidgets_LIBRARIES})
    target_include_directories(glance_core_tests PRIVATE src)
    add_test(NAME glance_core_tests COMMAND glance_core_tests)
endfunction()
