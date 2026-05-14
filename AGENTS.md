# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Project Overview
This is a C++ Markdown editor application built with wxWidgets and CMake. The application allows users to open folders, browse Markdown files, edit them in tabs, and view rendered previews side-by-side.

## Build System
- Language: C++17 or newer
- Framework: wxWidgets
- Build System: CMake
- Target Platforms: Linux, Windows, macOS

## Key Commands
- Build: `cmake . && make` (or `cmake --build .`)
- Run: `./glance` (after building)
- Command-line usage: `glance [folder_path]`

## Architecture
The application follows a component-based architecture with these key classes:
- `GlanceApp` - wxWidgets application entry point
- `MainFrame` - Main window, menus, status bar, splitters
- `FileTreePanel` - Folder tree and Markdown file browsing
- `EditorNotebook` - Manages editor tabs and open documents
- `GlanceCtrl` - Wrapper around wxStyledTextCtrl
- `PreviewPanel` - Renders and displays Markdown preview
- `MarkdownRenderer` - Converts Markdown to HTML
- `Document` - Represents one open Markdown document
- `DocumentManager` - Tracks open documents and save/close logic

## Code Style & Patterns
- C++17 or newer with modern C++ practices
- wxWidgets components used for UI elements
- Three-pane layout: file tree, editor tabs, preview
- File tree shows only Markdown-related files (.md, .markdown, .mdown, .mkd)
- Editor tabs show unsaved changes with asterisk prefix
- Preview updates with debounce (300-500ms) for performance
- Markdown rendering uses GitHub-Flavored Markdown support
- All file paths are resolved relative to the current Markdown file location

## Testing
- No specific test framework mentioned in documentation
- Tests should be organized in a tests/ directory with CMakeLists.txt
- Test files should be in same directory as source for proper test discovery

## Critical Gotchas
- Application must be built with CMake
- wxWidgets components must be properly linked
- Markdown rendering requires a Markdown parser (cmark-gfm recommended)
- Preview pane requires wxWebView for HTML rendering
- File paths in preview must be resolved relative to current file location
- Editor tabs must track document modification state properly
- Status bar must update caret position, selection, and document length