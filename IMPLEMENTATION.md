# Implementation Plan: Glance Markdown Editor

## Project Overview
This document outlines the detailed implementation plan for building the Glance Markdown editor, a portable desktop application using wxWidgets that allows users to open folders, browse Markdown files, edit them in tabs, and view rendered previews side-by-side.

## Development Approach
Following the incremental approach suggested in the project specification, we'll implement features in a logical sequence to build a working MVP before adding advanced features.

## Implementation Phases

### Phase 1: Basic Application Structure
**Goal**: Create a minimal working wxWidgets application with core UI components

1. **Create project structure**
   - Set up CMakeLists.txt with proper configuration
   - Create basic source directory structure
   - Set up build environment

2. **Implement main application entry point**
   - Create `GlanceApp` class inheriting from `wxApp`
   - Implement `OnInit()` method to create main window
   - Set up basic application initialization

3. **Build main window**
   - Create `MainFrame` class inheriting from `wxFrame`
   - Add menu bar with File, Edit, Format, Insert, Help menus
   - Add status bar
   - Implement basic window layout

### Phase 2: Folder and File Tree Browser
**Goal**: Implement folder browsing and Markdown file tree display

1. **Implement folder opening functionality**
   - Add "Open Folder" menu item and handler
   - Implement folder selection dialog
   - Handle folder validation and error cases

2. **Create file tree panel**
   - Implement `FileTreePanel` class
   - Use `wxTreeCtrl` or `wxDataViewTreeCtrl` for file display
   - Show only Markdown-related files (.md, .markdown, .mdown, .mkd)
   - Implement recursive folder traversal

3. **Integrate file tree with main window**
   - Add tree panel to three-pane layout
   - Implement event handling for file selection
   - Connect tree selection to editor functionality

### Phase 3: Tabbed Editor Implementation
**Goal**: Create tabbed editor with document management

1. **Implement editor notebook**
   - Create `EditorNotebook` class inheriting from `wxAuiNotebook` or `wxNotebook`
   - Implement tab management functionality
   - Handle tab creation and destruction

2. **Create editor control wrapper**
   - Implement `MarkdownEditorCtrl` class wrapping `wxStyledTextCtrl`
   - Add basic text editing features
   - Implement line numbers and syntax highlighting

3. **Document management**
   - Create `Document` class to represent open files
   - Implement `DocumentManager` to track open documents
   - Add modification state tracking
   - Implement tab title updates with asterisk indicators

### Phase 4: File Operations
**Goal**: Implement core file operations and save functionality

1. **Implement file creation**
   - [x] Add "New File" menu item and handler
   - [x] Implement file creation dialog
   - [x] Create empty Markdown files, refresh the tree, and open new files in tabs

2. **Implement file opening**
   - Add "Open File" menu item and handler
   - Implement file selection dialog
   - Handle file validation and loading

3. **Implement save functionality**
   - Add "Save File" and "Save All" menu items
   - Implement file saving logic
   - Update document modification state
   - Clear asterisk indicators on save

4. **Handle file closing**
   - Implement tab closing with modification prompts
   - Add "Close Tab" functionality
   - Handle unsaved changes appropriately

### Phase 5: Markdown Preview
**Goal**: Implement live Markdown preview functionality

1. **Create preview panel**
   - Implement `PreviewPanel` class
   - Use `wxWebView` for HTML rendering
   - Implement HTML template with proper styling

2. **Implement Markdown rendering**
   - Create `MarkdownRenderer` class
   - Integrate with Markdown parser (cmark-gfm recommended)
   - Handle GitHub-Flavored Markdown features

3. **Preview synchronization**
   - Implement preview update on document changes
   - Add debounce mechanism (300-500ms)
   - Synchronize preview with active editor tab

### Phase 6: Formatting and Insertion Tools
**Goal**: Add formatting and insertion capabilities

1. **Implement format menu**
   - Add formatting commands (bold, italic, headings, lists, etc.)
   - Implement text selection handling
   - Add placeholder insertion for commands

2. **Implement insert menu**
   - Add insertion commands (links, images, tables, code blocks)
   - Implement image path resolution
   - Add date/time insertion functionality

3. **Keyboard shortcuts**
   - Implement standard keyboard shortcuts
   - Map shortcuts to appropriate actions
   - Handle platform-specific key combinations

### Phase 7: Help and About System
**Goal**: Implement help documentation and about dialog

1. **Create help system**
   - Embed help.md file into binary
   - Implement help dialog with Markdown rendering
   - Add "Help" menu item

2. **Implement about dialog**
   - Create about dialog with application information
   - Include version, license, and wxWidgets version
   - Add "About" menu item

### Phase 8: Polish and Advanced Features
**Goal**: Add polish, error handling, and advanced features

1. **Error handling**
   - [x] Implement graceful error handling for file operations
   - [x] Add error dialogs for common failure cases
   - [x] Handle external file changes

2. **User experience improvements**
   - [x] Add window size and position persistence
   - [x] Implement recent files/folders functionality
   - [x] Add settings management

3. **Testing and documentation**
   - [x] Add unit tests for core functionality
   - [x] Document implementation details
   - [x] Create user documentation

### Phase 9: Further refining
**Goal**: Refine features

1. **Refining User experience**
   - [x] When no document is open no "Insert" or "Edit" menu items should be available. The same goes for "Save" menus.
   - [x] When adding Link ("Insert"/"Link") there should also be a text field for link text.
   - [x] Remove "Code Block", "Inline Code" and "Blockquote" from the "Insert" menu.
   - [x] When some text is selected and an "Insert" function is called the thing that is inserted shall be inserted before the selected text and NOT replacing the selection.
   - [x] Get a popup asking for how many columns when inserting a table.
   - [x] Remove "HTML Comment", "Footnote" and "Table Of Contents" from "Insert" menu.
   - [x] Remove "Underline" from "Format" menu.
   - [x] Update "resources/help.md" and "README.md" files.

## Technical Requirements

### Core Components
- **wxWidgets** for UI framework
- **C++17** or newer for modern C++ features
- **CMake** for build system
- **Markdown parser** (cmark-gfm recommended for GFM support)

### File Structure
```
glance/
├── CMakeLists.txt
├── README.md
├── resources/
│   ├── help.md
│   └── app_icon.png
├── src/
│   ├── main.cpp
│   ├── GlanceApp.h/cpp
│   ├── MainFrame.h/cpp
│   ├── FileTreePanel.h/cpp
│   ├── EditorNotebook.h/cpp
│   ├── GlanceCtrl.h/cpp
│   ├── PreviewPanel.h/cpp
│   ├── MarkdownRenderer.h/cpp
│   ├── Document.h/cpp
│   ├── DocumentManager.h/cpp
│   ├── HelpDialog.h/cpp
│   ├── AboutDialog.h/cpp
│   └── CommandLineOptions.h/cpp
└── tests/
    └── CMakeLists.txt
```

## Implementation Timeline

### Week 1: Foundation
- Project setup and basic application structure
- Main window with menu bar and status bar
- Folder opening and basic UI layout

### Week 2: File Tree and Editor
- File tree browser implementation
- Tabbed editor with basic editing capabilities
- Document management system

### Week 3: File Operations and Preview
- File save/load functionality
- Markdown preview system
- Synchronization between editor and preview

### Week 4: Formatting and Features
- Formatting menu implementation
- Insertion commands
- Keyboard shortcuts

### Week 5: Help and Polish
- Help system and about dialog
- Error handling and user experience improvements
- Testing and documentation

## Testing Strategy

1. **Unit Tests**
   - Test individual components (Document, DocumentManager, MarkdownRenderer)
   - Test edge cases for file operations
   - Test formatting and insertion logic

2. **Integration Tests**
   - Test full workflow from folder opening to file editing
   - Test preview synchronization
   - Test menu interactions

3. **User Acceptance Tests**
   - Verify all acceptance criteria from PROJECT.md
   - Test cross-platform compatibility
   - Validate user experience and workflow

## Risk Mitigation

1. **Build System Issues**
   - Use CMake for cross-platform compatibility
   - Test on all target platforms early

2. **wxWidgets Integration**
   - Start with minimal wxWidgets features
   - Gradually add complexity

3. **Markdown Rendering**
   - Select a mature Markdown parser
   - Test with various Markdown syntax examples

4. **Performance**
   - Implement debounce for preview updates
   - Profile memory usage for large documents
