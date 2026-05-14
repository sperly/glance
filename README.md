# Glance Markdown Editor

A portable desktop application for editing Markdown files, built with wxWidgets.

## Project Structure

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

## Implementation Status

### Phase 1: Basic Application Structure
- [x] Project structure created
- [x] CMakeLists.txt configured
- [x] Main application entry point
- [x] GlanceApp class implemented
- [x] MainFrame class with menu bar and status bar
- [x] Basic menu items (File, Help)

### Phase 2: Folder and File Tree Browser
- [x] Folder opening functionality
- [x] File tree panel implementation
- [x] Integration with main window

### Phase 3: Tabbed Editor Implementation
- [x] Editor notebook implementation
- [x] Editor control wrapper
- [x] Document management system

### Phase 4: File Operations
- [x] New file functionality
- [x] File opening functionality
- [x] Save functionality
- [x] File closing handling

### Phase 5: Markdown Preview
- [x] Preview panel implementation
- [x] Markdown rendering system
- [x] Preview synchronization

### Phase 6: Formatting and Insertion Tools
- [x] Format menu implementation
- [x] Insert menu implementation
- [x] Keyboard shortcuts

### Phase 7: Help and About System
- [x] Help system implementation
- [x] About dialog implementation

### Phase 8: Polish and Advanced Features
- [x] Error handling
- [x] User experience improvements
- [x] Testing and documentation

## Building and Testing

Configure and build the application with CMake:

```sh
cmake -S . -B build
cmake --build build
```

Run the lightweight core test suite with CTest:

```sh
ctest --test-dir build --output-on-failure
```

## User Notes

- Create a document with **File > New File...**.
- Open a workspace with **File > Open Folder...** or a single Markdown file with **File > Open File...**.
- Recently opened files and folders are available from the **File** menu.
- Window size, splitter positions, and recent items are restored between launches.
- When an open document changes on disk while Glance is in the background, Glance prompts before reloading it.
- Use **Help > Save Preview HTML...** to export the current preview as raw HTML.
