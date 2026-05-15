# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Setup & Development

- Build with CMake: `cmake -S . -B build && cmake --build build`
- Run tests with CTest: `ctest --test-dir build --output-on-failure`
- Use `cmake --build build` to rebuild after changes

## Testing

- Core tests are implemented in `tests/core_tests.cpp` using wxWidgets unit testing approach
- Tests use `wxInitializer` for wxWidgets initialization
- Test files must be in same directory as source for CTest to work properly
- Tests are run with CTest, not standard npm test commands

## Architecture

- This is a C++ desktop application using wxWidgets framework
- Main application entry point is in `src/main.cpp` with `wxIMPLEMENT_APP(GlanceApp)`
- Core components include: `GlanceApp`, `MainFrame`, `FileTreePanel`, `EditorNotebook`, `DocumentManager`, `MarkdownRenderer`
- Uses C++17 standard with wxWidgets libraries (core, base, adv, aui, html, stc)
- Markdown rendering uses custom implementation with regex-based parsing
- Preview panel uses `wxWebView` for HTML rendering

## Framework Quirks

- Uses CMake build system, not npm/yarn
- All source files are C++ (.cpp/.h) with wxWidgets UI components
- Markdown rendering is custom implementation, not using external libraries
- File paths are resolved using wxWidgets `wxFileName` class
- Uses wxWidgets' `wxStyledTextCtrl` for text editing with syntax highlighting
- Uses `wxWebView` for preview rendering with proper HTML template

## Code Style

- C++17 with modern C++ features
- Uses wxWidgets naming conventions (camelCase for methods, PascalCase for classes)
- All source files use UTF-8 encoding
- Custom Markdown renderer uses regex patterns for parsing
- File paths are normalized using wxWidgets path handling utilities
- Uses `wxString` for all string handling, converting to/from std::string with `.utf8_string()`

## Gotchas

- Markdown rendering uses custom implementation with specific HTML output format
- Preview synchronization uses debounce mechanism (300-500ms)
- File tree only shows Markdown-related files (.md, .markdown, .mdown, .mkd)
- Image paths in Markdown are resolved relative to the source file's directory
- Task items in Markdown use specific HTML classes for styling (task, glance-strike)
- All file operations use wxWidgets file handling utilities for cross-platform compatibility
- Document management normalizes duplicate file paths using `wxFileName::GetFullPath()`
- Test files must be in same directory as source for CTest to work properly