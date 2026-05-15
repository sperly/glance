cmake_minimum_required(VERSION 3.10)
project(glance VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GlanceResources)
include(GlanceTests)
include(GlanceWebView)

set(GLANCE_APP_SOURCES
    src/main.cpp
    src/EmbeddedResources.h
    src/GlanceApp.cpp
    src/GlanceApp.h
    src/AboutDialog.cpp
    src/AboutDialog.h
    src/MainFrame.cpp
    src/MainFrame.h
    src/FileTreePanel.cpp
    src/FileTreePanel.h
    src/Document.cpp
    src/Document.h
    src/DocumentManager.cpp
    src/DocumentManager.h
    src/EditorNotebook.cpp
    src/EditorNotebook.h
    src/GlanceCtrl.cpp
    src/GlanceCtrl.h
    src/HelpDialog.cpp
    src/HelpDialog.h
    src/MarkdownRenderer.cpp
    src/MarkdownRenderer.h
    src/PreviewPanel.cpp
    src/PreviewPanel.h
    src/ResourcePaths.cpp
    src/ResourcePaths.h
    src/SettingsManager.cpp
    src/SettingsManager.h
)

set(GLANCE_FORMAT_SOURCES
    ${GLANCE_APP_SOURCES}
    tests/core_tests.cpp
)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base adv aui html stc)
include(${wxWidgets_USE_FILE})

glance_configure_embedded_resources(GLANCE_EMBEDDED_RESOURCES_SOURCE)

# Add executable
add_executable(glance WIN32
    ${GLANCE_APP_SOURCES}
    ${GLANCE_EMBEDDED_RESOURCES_SOURCE}
)

# Link wxWidgets libraries
target_link_libraries(glance ${wxWidgets_LIBRARIES})
glance_configure_webview(glance)

target_compile_definitions(glance
    PRIVATE
        GLANCE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        GLANCE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        GLANCE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        GLANCE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        GLANCE_VERSION_STRING="${PROJECT_VERSION}"
)

# Include directories
target_include_directories(glance PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${GLANCE_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting C++ sources with clang-format"
        VERBATIM
    )
else()
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Install clang-format to use the format target."
        VERBATIM
    )
endif()

glance_add_core_tests()
