# Project Specification: Glance, MarkDown Editor

## 1. Project Overview

Build a portable desktop Markdown editor using the **wxWidgets** UI framework. The application shall allow users to open a folder from the command line or through the GUI, browse Markdown files in that folder, edit multiple Markdown files in tabs, and view a rendered Markdown preview side-by-side with the raw editor.

The goal is to create a practical, lightweight Markdown writing environment with a native desktop feel and good cross-platform behavior on Linux, Windows, and macOS.

## 2. Primary Goals

- Use **wxWidgets** as the main GUI framework.
- Support opening a folder from the command line.
- Display Markdown files from the opened folder in a tree browser.
- Allow opening and editing multiple Markdown files in tabs.
- Show a live or refreshable Markdown preview.
- Track unsaved changes clearly.
- Provide menus for file operations, editing, formatting, insertion, help, and application information.
- Provide a useful status bar with document and selection information.
- Keep the architecture clean enough for future extension.

## 3. Target Platforms

The application should be designed for portability and should build on:

- Linux
- Windows
- macOS

Platform-specific code should be minimized and isolated where unavoidable.

## 4. Technology Requirements

### 4.1 UI Framework

The application shall be based on **wxWidgets**.

Recommended wxWidgets components:

- `wxApp`
- `wxFrame`
- `wxMenuBar`
- `wxMenu`
- `wxStatusBar`
- `wxSplitterWindow`
- `wxTreeCtrl` or `wxDataViewTreeCtrl`
- `wxAuiNotebook` or `wxNotebook`
- `wxStyledTextCtrl` for the raw Markdown editor
- `wxWebView` for rendered preview, if available
- `wxFileDialog`
- `wxDirDialog`
- `wxPrintout` / `wxPrintPreview` where applicable

### 4.2 Language

The preferred implementation language is **C++17 or newer**.

### 4.3 Build System

Use **CMake** as the build system.

The project should include:

- Top-level `CMakeLists.txt`
- Source directory
- Resource or embedded-data handling for help Markdown
- Clear build instructions in `README.md`

## 5. Command-Line Behavior

The application shall support being launched with an optional folder argument.

### 5.1 Examples

```bash
markdown-editor
markdown-editor /path/to/project
markdown-editor .
```

### 5.2 Rules

- If no argument is provided, the application starts with no folder opened.
- If a valid folder path is provided, the application opens that folder on startup.
- If an invalid path is provided, the application shows an error dialog and starts without an opened folder.
- If a file path is provided instead of a folder, the application may either:
  - open the file directly, or
  - show an error explaining that a folder was expected.

The recommended behavior is to support both folder and file arguments:

- Folder argument: open the folder and populate the Markdown file tree.
- Markdown file argument: open its parent folder and open the file in an editor tab.

## 6. Main Window Layout

The main window shall have three resizable vertical sections:

```text
+--------------------+-------------------------------+---------------------------+
| Markdown File Tree | Raw Markdown Editor Tabs      | Markdown Preview          |
|                    |                               |                           |
| Left Pane          | Center Pane                   | Right Pane                |
+--------------------+-------------------------------+---------------------------+
| Status Bar                                                                     |
+--------------------------------------------------------------------------------+
```

### 6.1 Left Pane: File Tree Browser

The left pane shall display all Markdown files in the currently opened folder.

Requirements:

- Show files recursively from the opened folder.
- Show only Markdown-related files by default:
  - `.md`
  - `.markdown`
  - `.mdown`
  - `.mkd`
- Preserve folder hierarchy.
- Allow opening a file by double-clicking or pressing Enter.
- Refresh when a new folder is opened.
- Optional future enhancement: detect external file changes and refresh the tree automatically.

The file tree should not show hidden files or hidden folders by default, unless this is later made configurable.

### 6.2 Center Pane: Raw Markdown Editor

The center pane shall contain a tabbed editor area.

Requirements:

- Each opened Markdown file appears in its own tab.
- Tabs show the file name.
- Unsaved files shall show an asterisk before the file name.

Example:

```text
README.md
*notes.md
```

- The editor shall allow normal text editing.
- The editor should use a monospaced font by default.
- The editor should support common editing shortcuts:
  - Ctrl+S: Save current file
  - Ctrl+Shift+S: Save all files
  - Ctrl+O: Open file
  - Ctrl+Shift+O or Ctrl+Alt+O: Open folder
  - Ctrl+W: Close current tab
  - Ctrl+P: Print
  - Ctrl+Z: Undo
  - Ctrl+Y or Ctrl+Shift+Z: Redo
  - Ctrl+X: Cut
  - Ctrl+C: Copy
  - Ctrl+V: Paste

Recommended editor component:

- `wxStyledTextCtrl`

Recommended editor features:

- Line numbers
- Current line highlight
- Basic Markdown syntax highlighting
- Undo/redo
- Selection tracking
- Caret position tracking
- Configurable tab width
- Soft wrapping toggle, either from a future View menu or settings

### 6.3 Right Pane: Markdown Preview

The right pane shall display a rendered preview of the currently selected editor tab.

Requirements:

- The preview updates when the active tab changes.
- The preview should update when the document changes.
- The update may be live with a debounce delay, for example 300–500 ms.
- If live preview is too expensive initially, a manual refresh action may be implemented first.
- The preview should render common Markdown features:
  - Headings
  - Paragraphs
  - Emphasis
  - Lists
  - Code blocks
  - Blockquotes
  - Tables
  - Links
  - Images
  - Horizontal rules
  - Task lists, if supported by the Markdown renderer

Recommended preview component:

- `wxWebView`

The Markdown should be converted to HTML internally and displayed in the preview pane.

## 7. Window Resizing and Splitters

The three main areas must be resizable by the user.

Recommended implementation:

- Use nested `wxSplitterWindow` controls, or
- Use `wxAuiManager` if a more dockable/flexible UI is desired.

Initial size distribution:

- Left tree browser: 20%
- Center editor: 50%
- Right preview: 30%

The application should remember the last splitter positions if settings persistence is implemented.

## 8. Menu Structure

The application shall have a menu bar with five top-level menus:

1. File
2. Edit
3. Format
4. Insert
5. Help

## 9. File Menu

The **File** menu shall contain:

| Menu Item | Description |
|---|---|
| Open Folder | Opens a folder and populates the Markdown tree browser. |
| Open File | Opens a Markdown file in a new editor tab. |
| Save File | Saves the currently active file. |
| Save All | Saves all modified open files. |
| Print | Prints the current document or rendered preview. |
| Exit | Closes the application. |

### 9.1 File Menu Behavior

#### Open Folder

- Shows a folder picker dialog.
- Clears or replaces the current file tree.
- If unsaved files are open, ask the user whether to save, discard, or cancel before switching folders.

#### Open File

- Shows a file picker dialog.
- Accepts Markdown files by default.
- Opens the selected file in a new tab.
- If the file is already open, switch to its existing tab.

#### Save File

- Saves the current active editor tab.
- Clears the unsaved asterisk from the tab title.
- Updates status bar state.

#### Save All

- Saves all modified open editor tabs.
- Clears unsaved asterisks from saved tabs.
- Reports failure if any file could not be saved.

#### Print

The initial implementation may print either:

- the raw Markdown text, or
- the rendered preview.

Preferred behavior:

- Print rendered preview if preview rendering is implemented.
- Otherwise print raw Markdown as plain text.

#### Exit

- If there are unsaved documents, prompt the user to save, discard, or cancel.
- Close the application only after all unsaved-state handling is resolved.

## 10. Edit Menu

The **Edit** menu shall contain:

| Menu Item | Description |
|---|---|
| Undo | Undo the last editing operation. |
| Redo | Redo the last undone operation. |
| Cut | Cut selected text. |
| Copy | Copy selected text. |
| Paste | Paste text from clipboard. |
| Select All | Select all text in the current editor. |
| Find | Search in the current document. |
| Find Next | Move to the next search match. |
| Find Previous | Move to the previous search match. |
| Replace | Replace text in the current document. |

Minimum required items from the original request:

- Cut
- Copy
- Paste

Recommended additional edit actions:

- Undo
- Redo
- Select All
- Find
- Replace

## 11. Format Menu

The **Format** menu shall contain Markdown formatting tools that operate on the selected text or insert Markdown syntax at the caret position.

| Menu Item | Markdown Action |
|---|---|
| Bold | Wrap selected text with `**text**`. |
| Italic | Wrap selected text with `*text*`. |
| Bold Italic | Wrap selected text with `***text***`. |
| Underline | Insert HTML underline: `<u>text</u>`. Markdown has no standard underline syntax. |
| Strikethrough | Wrap selected text with `~~text~~`. |
| Inline Code | Wrap selected text with backticks. |
| Code Block | Insert fenced code block using triple backticks. |
| Blockquote | Prefix selected lines with `> `. |
| Heading 1 | Prefix line with `# `. |
| Heading 2 | Prefix line with `## `. |
| Heading 3 | Prefix line with `### `. |
| Heading 4 | Prefix line with `#### `. |
| Heading 5 | Prefix line with `##### `. |
| Heading 6 | Prefix line with `###### `. |
| Bullet List | Prefix selected lines with `- `. |
| Numbered List | Prefix selected lines with `1.`, `2.`, `3.`, etc. |
| Task List | Prefix selected lines with `- [ ] `. |
| Completed Task | Prefix selected lines with `- [x] `. |
| Horizontal Rule | Insert `---` on its own line. |
| Clear Formatting | Remove common Markdown formatting markers where practical. |

### 11.1 Format Behavior

Formatting commands should follow these rules:

- If text is selected, wrap or transform the selected text.
- If no text is selected, insert a useful placeholder.
- For line-based commands, operate on the current line or selected lines.
- Preserve selection where reasonable.
- Mark the document as modified after applying formatting.
- Update the preview after applying formatting.

### 11.2 Placeholder Examples

| Command | Inserted Placeholder |
|---|---|
| Bold | `**bold text**` |
| Italic | `*italic text*` |
| Link | `[link text](https://example.com)` |
| Image | `![alt text](image.png)` |
| Code Block | fenced code block with placeholder content |
| Table | a small sample Markdown table |

## 12. Insert Menu

The **Insert** menu shall contain insertable Markdown structures and common document elements.

| Menu Item | Inserted Markdown |
|---|---|
| Link | `[link text](https://example.com)` |
| Image | `![alt text](image.png)` |
| Table | A Markdown table template. |
| Code Block | A fenced code block. |
| Inline Code | Inline backtick code. |
| Blockquote | A blockquote template. |
| Bullet List | A bullet list template. |
| Numbered List | A numbered list template. |
| Task List | A task list template. |
| Horizontal Rule | `---` |
| Date | Current date in ISO format, for example `2026-05-14`. |
| Time | Current local time. |
| Date and Time | Current local date and time. |
| HTML Comment | `<!-- comment -->` |
| Footnote | Footnote reference and definition, if supported by renderer. |
| Table of Contents Marker | Insert a marker such as `[TOC]`, if supported by renderer or future extension. |

### 12.1 Insert Image Behavior

The Image command should:

- Open a file picker for image files.
- Support common image formats:
  - `.png`
  - `.jpg`
  - `.jpeg`
  - `.gif`
  - `.svg`
  - `.webp`
- Insert a relative path when the image is inside the opened folder.
- Insert an absolute path only if no useful relative path is possible.
- Ask for or infer alt text.

Example inserted Markdown:

```markdown
![Alt text](images/example.png)
```

### 12.2 Insert Table Behavior

The Table command should insert a simple default table:

```markdown
| Column 1 | Column 2 | Column 3 |
|---|---|---|
| Value 1 | Value 2 | Value 3 |
```

A future enhancement may provide a dialog where the user selects the number of rows and columns.

### 12.3 Insert Link Behavior

The Link command should:

- If text is selected, use the selected text as link text.
- Prompt for the URL.
- Insert:

```markdown
[selected text](https://example.com)
```

If no text is selected, insert:

```markdown
[link text](https://example.com)
```

## 13. Help Menu

The **Help** menu shall contain:

| Menu Item | Description |
|---|---|
| Help | Opens an extensive guide to the application. |
| About | Shows application name, version, license, and basic information. |

### 13.1 Help System

The Help page should be generated from a Markdown file embedded at compile time.

Requirements:

- Store the help source as Markdown, for example `resources/help.md`.
- Embed this file into the binary during compilation.
- Render the Markdown help as HTML in a help window using the same Markdown rendering system as the preview pane.
- The help window may be a modal or modeless dialog.
- The help content should explain:
  - Opening folders
  - Opening files
  - Editing files
  - Saving files
  - Using the preview
  - Using Markdown formatting commands
  - Inserting images, links, and tables
  - Keyboard shortcuts
  - Unsaved file indicators
  - Printing
  - Troubleshooting

### 13.2 About Dialog

The About dialog shall show:

- Application name
- Version
- Short description
- Author or organization
- License
- wxWidgets version
- Optional build information

## 14. Status Bar

A status bar shall be displayed at the bottom of the main window.

It should show useful information about the active document.

Recommended fields:

| Field | Description |
|---|---|
| Current File | Current file name or path. |
| Modified State | Saved or modified. |
| Caret Position | Current line and column. |
| Selection | Number of selected characters. |
| Document Length | Total number of characters. |
| Encoding | Example: UTF-8. |
| Line Endings | Example: LF or CRLF. |

Example status text:

```text
README.md | Modified | Line 24, Column 8 | Selected: 12 chars | Length: 4,812 chars | UTF-8 | LF
```

Minimum required information:

- Number of selected characters
- Current position in document
- Total document length

## 15. Document State Management

Each open document shall track:

- File path
- Display name
- Raw text content
- Modified state
- Last saved content or modification timestamp
- Associated editor control
- Associated tab index

### 15.1 Unsaved Indicator

When a document is modified:

- Mark it as dirty.
- Prefix the tab title with `*`.

When a document is saved:

- Clear the dirty state.
- Remove the `*` prefix.

### 15.2 Closing Modified Documents

When closing a modified document, show a prompt:

```text
The file "notes.md" has unsaved changes.
Do you want to save them before closing?
```

Options:

- Save
- Discard
- Cancel

### 15.3 External File Changes

Initial implementation may ignore external changes.

Recommended future behavior:

- Detect if an opened file has changed on disk.
- Ask whether to reload, keep editor version, or compare.

## 16. Markdown Rendering

The editor needs a Markdown-to-HTML rendering component.

Possible renderer choices:

- `cmark`
- `cmark-gfm`
- `md4c`
- another suitable C/C++ Markdown parser

Recommended:

- Use a renderer that supports GitHub-Flavored Markdown if possible.
- Tables and task lists are desirable.
- Avoid writing a Markdown parser from scratch.

### 16.1 Preview HTML

The rendered preview should be wrapped in a simple HTML template:

```html
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <style>
    body {
      font-family: sans-serif;
      margin: 2rem;
      line-height: 1.5;
    }
    code {
      font-family: monospace;
    }
    pre {
      padding: 1rem;
      overflow-x: auto;
    }
    img {
      max-width: 100%;
    }
  </style>
</head>
<body>
  <!-- Rendered Markdown goes here -->
</body>
</html>
```

### 16.2 Image Path Handling

The preview must resolve relative image paths relative to the current Markdown file location.

Example:

```markdown
![Diagram](images/diagram.png)
```

If the current file is:

```text
/project/docs/readme.md
```

then the image path should resolve to:

```text
/project/docs/images/diagram.png
```

## 17. Keyboard Shortcuts

Recommended default shortcuts:

| Shortcut | Action |
|---|---|
| Ctrl+O | Open File |
| Ctrl+Shift+O | Open Folder |
| Ctrl+S | Save File |
| Ctrl+Shift+S | Save All |
| Ctrl+P | Print |
| Ctrl+W | Close Current Tab |
| Ctrl+Q | Exit |
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| Ctrl+X | Cut |
| Ctrl+C | Copy |
| Ctrl+V | Paste |
| Ctrl+A | Select All |
| Ctrl+F | Find |
| Ctrl+H | Replace |
| Ctrl+B | Bold |
| Ctrl+I | Italic |
| Ctrl+Shift+X | Strikethrough |
| Ctrl+K | Insert Link |
| Ctrl+Shift+I | Insert Image |

On macOS, standard Command-key equivalents should be used where wxWidgets provides native accelerator behavior.

## 18. Suggested Architecture

The implementation should be separated into clear components.

### 18.1 Suggested Classes

| Class | Responsibility |
|---|---|
| `MarkdownEditorApp` | wxWidgets application entry point. |
| `MainFrame` | Main window, menus, status bar, splitters. |
| `FileTreePanel` | Folder tree and Markdown file browsing. |
| `EditorNotebook` | Manages editor tabs and open documents. |
| `MarkdownEditorCtrl` | Wrapper around `wxStyledTextCtrl`. |
| `PreviewPanel` | Renders and displays Markdown preview. |
| `MarkdownRenderer` | Converts Markdown to HTML. |
| `Document` | Represents one open Markdown document. |
| `DocumentManager` | Tracks open documents and save/close logic. |
| `CommandLineOptions` | Parses startup arguments. |
| `HelpDialog` | Displays embedded Markdown help. |
| `AboutDialog` | Displays application information. |
| `SettingsManager` | Optional persistence for window layout and preferences. |

### 18.2 Data Flow

When a file is selected in the tree:

1. `FileTreePanel` emits an open-file request.
2. `DocumentManager` checks if the file is already open.
3. If already open, `EditorNotebook` activates the existing tab.
4. If not open, the file is loaded from disk.
5. A new editor tab is created.
6. The preview panel renders the active document.
7. The status bar updates.

When text changes in the editor:

1. The editor emits a document-changed event.
2. The document is marked modified.
3. The tab title receives an asterisk.
4. The status bar updates.
5. The preview update is scheduled with a debounce timer.

## 19. Persistence and Settings

Initial settings may be optional, but the design should allow adding them.

Recommended persisted settings:

- Last opened folder
- Window size and position
- Splitter positions
- Recently opened files
- Recently opened folders
- Editor font
- Editor tab width
- Word wrap enabled/disabled
- Preview update mode

Settings may be stored using:

- `wxConfig`
- a simple configuration file
- platform-native settings storage through wxWidgets

## 20. Error Handling

The application shall handle errors gracefully.

Examples:

- Cannot open folder
- Cannot read file
- Cannot save file
- File deleted while open
- Markdown rendering failure
- Image path not found
- Print failure

Errors should be shown using dialogs and should not crash the application.

## 21. Minimum Viable Product

The first usable version should include:

- wxWidgets main window
- Command-line folder opening
- Three-pane layout
- Markdown file tree
- Tabbed raw editor
- Unsaved asterisk indicator
- Save current file
- Save all files
- Rendered preview
- File, Edit, Format, Insert, and Help menus
- Status bar with caret position, selected character count, and total document length
- Embedded Markdown help page
- About dialog

## 22. Future Enhancements

Possible future improvements:

- Recent files and recent folders menu
- Auto-save
- Spell checking
- Export to HTML
- Export to PDF
- Configurable CSS for preview
- Dark mode
- Synchronized editor/preview scrolling
- Mermaid diagram support
- Math rendering with KaTeX or MathJax
- Git status indicators in file tree
- Find in folder
- Outline panel generated from Markdown headings
- Drag-and-drop file opening
- Drag-and-drop image insertion
- User-configurable keyboard shortcuts
- Plugin system

## 23. Acceptance Criteria

The implementation is considered complete when the following are true:

- The application builds with CMake on at least one target platform.
- The application starts successfully without command-line arguments.
- The application can be launched with a folder path argument.
- The folder tree displays Markdown files from the opened folder.
- Double-clicking a Markdown file opens it in an editor tab.
- Multiple files can be open at once.
- Editing a file marks the tab with an asterisk.
- Saving the file removes the asterisk.
- Save All saves all modified documents.
- The preview pane displays rendered Markdown for the active tab.
- Switching tabs updates the preview.
- The status bar updates caret position, selection length, and total document length.
- File menu actions work.
- Edit menu actions work.
- Format menu inserts or applies Markdown syntax.
- Insert menu inserts common Markdown structures.
- Help opens an embedded Markdown-based guide.
- About shows application information.
- The application prompts before losing unsaved changes.

## 24. Suggested Initial Repository Layout

```text
markdown-editor/
├── CMakeLists.txt
├── README.md
├── resources/
│   ├── help.md
│   └── app_icon.png
├── src/
│   ├── main.cpp
│   ├── MarkdownEditorApp.h
│   ├── MarkdownEditorApp.cpp
│   ├── MainFrame.h
│   ├── MainFrame.cpp
│   ├── FileTreePanel.h
│   ├── FileTreePanel.cpp
│   ├── EditorNotebook.h
│   ├── EditorNotebook.cpp
│   ├── MarkdownEditorCtrl.h
│   ├── MarkdownEditorCtrl.cpp
│   ├── PreviewPanel.h
│   ├── PreviewPanel.cpp
│   ├── MarkdownRenderer.h
│   ├── MarkdownRenderer.cpp
│   ├── Document.h
│   ├── Document.cpp
│   ├── DocumentManager.h
│   ├── DocumentManager.cpp
│   ├── HelpDialog.h
│   ├── HelpDialog.cpp
│   ├── AboutDialog.h
│   └── AboutDialog.cpp
└── tests/
    └── CMakeLists.txt
```

## 25. Notes for AI Implementation

When implementing this project, prefer a clean incremental approach:

1. Create a minimal wxWidgets application with a main frame.
2. Add menu bar and status bar.
3. Add the three-pane splitter layout.
4. Add folder opening and Markdown file tree.
5. Add tabbed editor support.
6. Add document modified tracking.
7. Add save and save-all behavior.
8. Add Markdown rendering and preview.
9. Add formatting and insertion commands.
10. Add embedded help.
11. Add polish, keyboard shortcuts, error handling, and persistence.

Avoid over-engineering the first version. The first implementation should prioritize a working editor with clean structure over advanced features.
