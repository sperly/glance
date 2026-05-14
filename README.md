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
- [ ] Folder opening functionality
- [ ] File tree panel implementation
- [ ] Integration with main window

### Phase 3: Tabbed Editor Implementation
- [ ] Editor notebook implementation
- [ ] Editor control wrapper
- [ ] Document management system

### Phase 4: File Operations
- [ ] File opening functionality
- [ ] Save functionality
- [ ] File closing handling

### Phase 5: Markdown Preview
- [ ] Preview panel implementation
- [ ] Markdown rendering system
- [ ] Preview synchronization

### Phase 6: Formatting and Insertion Tools
- [ ] Format menu implementation
- [ ] Insert menu implementation
- [ ] Keyboard shortcuts

### Phase 7: Help and About System
- [ ] Help system implementation
- [ ] About dialog implementation

### Phase 8: Polish and Advanced Features
- [ ] Error handling
- [ ] User experience improvements
- [ ] Testing and documentation