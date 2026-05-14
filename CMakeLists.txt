cmake_minimum_required(VERSION 3.10)
project(glance)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base adv aui html)
include(${wxWidgets_USE_FILE})

# Add executable
add_executable(glance
    src/main.cpp
    src/GlanceApp.cpp
    src/GlanceApp.h
    src/MainFrame.cpp
    src/MainFrame.h
)

# Link wxWidgets libraries
target_link_libraries(glance ${wxWidgets_LIBRARIES})

# Include directories
target_include_directories(glance PRIVATE src)