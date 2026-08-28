# Glance Markdown Editor
[![CMake on multiple platforms](https://github.com/sperly/glance/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/sperly/glance/actions/workflows/cmake-multi-platform.yml)

![glance-logo](resources/glance-logo-small.png)

Located at [Glance at Github](https://github.com/sperly/glance)

A portable desktop application for editing Markdown files, built with wxWidgets.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Project Structure

```
glance/
├── CMakeLists.txt
├── README.md
├── resources/
│   ├── help.md
│   └── glance-icon.png
│   └── glance-logo.png
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
│   ├── DocumentSettingsDialog.h/cpp
│   ├── MarkdownFlavor.h/cpp
│   ├── MarkdownValidator.h/cpp
│   └── SettingsManager.h/cpp
└── tests/
    └── core_tests.cpp
```

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

Format the C++ sources with clang-format using the project Google-style configuration:

```sh
cmake --build build --target format
```

Check formatting without modifying files:

```sh
cmake --build build --target format-check
```

## Documentation

Additional developer documentation lives in `docs/`:

- [Codebase overview](docs/codebase-overview.md)
- [Adding a Markdown flavor](docs/adding-markdown-flavor.md)

## Features

Glance is a portable Markdown editor with a folder tree, tabbed editor, and rendered preview.

### Opening Folders

Use `File > Open Folder` to choose a folder. Glance scans the folder recursively and shows Markdown files in the left tree.

Supported file types:

- `.md`
- `.markdown`
- `.mdown`
- `.mkd`

Hidden files and folders are skipped by default.

### Opening Files

Open a file by double-clicking it in the file tree, or use `File > Open File`. If the file is already open, Glance switches to the existing tab.

You can also launch Glance with a folder or Markdown file path:

```text
glance /path/to/folder
glance /path/to/file.md
```

### Editing And Saving

Each Markdown file opens in its own tab. Modified files show an asterisk before the tab name.

Use:

- `File > Save File` to save the current tab.
- `File > Save All` to save all modified tabs.
- `File > Close Tab` to close the current tab.
- `Edit > Search and Replace` to find text, replace the current match, or replace all matches in the current tab.

When closing a modified tab, switching folders, or exiting the app, Glance asks whether to save changes.
Document editing, insertion, saving, closing, preview export, and printing commands are disabled when no document is open.

### Document Markdown Flavor

Use `Document > Settings` to choose the Markdown flavor for the current document. Glance uses `GitHub Markdown` by default and also supports `Vanilla Markdown`.

The selected flavor controls preview rendering, printing, preview HTML export, validation, and which Markdown formatting or insertion commands are available. Commands that do not apply to the current document flavor are hidden.

Use `Document > Validate Markdown` to check the current document against its selected flavor.

### Preview

The right pane shows a rendered preview of the active Markdown tab. The preview updates automatically after a short debounce when you type.

In GitHub Markdown mode, the preview supports common Markdown structures:

- Headings
- Paragraphs
- Emphasis and strong text
- Strikethrough
- Subscript and superscript
- Lists and task lists
- Fenced code blocks and inline code
- PlantUML diagrams when the `plantuml` executable is available
- Mermaid diagrams when the `mmdc` or `mermaid` executable is available
- Blockquotes
- Tables
- Links and images
- Horizontal rules

Backslash escapes are honored for Markdown special characters, so escaped
syntax such as `\#`, `\*text\*`, `\[label\]\(target\)`, and `\- item`
renders as literal text instead of headings, emphasis, links, or lists. Escaped
spaces are preserved in the preview.

Vanilla Markdown mode supports the core Markdown subset without GitHub-specific extensions such as tables, task lists, strikethrough, subscript, and superscript.

Image paths are resolved relative to the Markdown file location when possible.

### Preview And Source Synchronization

The editor and the preview follow each other when `View > Follow Source` is
checked, which is the default. Uncheck it to stop the two panes tracking each
other.

- Clicking a block in the preview moves the editor caret to the Markdown line that produced it. Focus stays in the preview so you can keep reading or selecting text.
- Moving the caret in the editor highlights the matching block in the preview and scrolls it into view when it is off screen.

The preview also restores this position after it re-renders, so it no longer jumps back to the top while you type.

Synchronization requires the `wxWebView` preview engine. With the `wxHtmlWindow` fallback the preview still renders, but it cannot follow the caret or report clicks.

### Formatting Commands

Use the `Format` menu to transform selected text or insert placeholders at the caret.

Available commands include:

- Bold
- Italic
- Bold Italic
- Strikethrough
- Subscript
- Superscript
- Inline Code
- Code Block
- Blockquote
- Heading 1 through Heading 6
- Bullet List
- Numbered List
- Task List
- Completed Task
- Horizontal Rule
- Clear Formatting

Line-based commands operate on the current line or selected lines.
Some commands are disabled when the current document flavor does not define the required Markdown tag.

### Insert Commands

Use the `Insert` menu to add Markdown snippets:

- Link
- Image
- Table
- PlantUML Diagram
- Mermaid Diagram
- Bullet List
- Numbered List
- Task List
- Horizontal Rule
- Date
- Time
- Date and Time

The link command prompts for link text and URL. The image command prompts for an image file and alt text. Glance inserts a relative path when the image is near the current Markdown file. The table command creates a simple 3-column, 3-row table with placeholder text.

PlantUML diagrams use fenced blocks such as:

````text
```plantuml
@startuml
Alice -> Bob: Hello
@enduml
```
````

Glance looks for `plantuml` on `PATH` when rendering the preview. When it is
not available, or when PlantUML cannot render the diagram source, the preview
displays `No PlantUML Support or error in PlantUML code` in a code box.

Mermaid diagrams use fenced blocks such as:

````text
```mermaid
flowchart TD
    A[Start] --> B[End]
```
````

Glance looks for `mmdc` or `mermaid` on `PATH` when rendering the preview.
When it is not available, or when Mermaid cannot render the diagram source,
the preview displays `No Mermaid Support or error in Mermaid code` in a
code box.

If text is selected, inserted snippets are placed before the selected text instead of replacing it.

### Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+O` | Open File |
| `Ctrl+Shift+O` | Open Folder |
| `Ctrl+S` | Save File |
| `Ctrl+Shift+S` | Save All |
| `Ctrl+W` | Close Tab |
| `Ctrl+P` | Print |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Ctrl+X` | Cut |
| `Ctrl+C` | Copy |
| `Ctrl+V` | Paste |
| `Ctrl+A` | Select All |
| `Ctrl+H` | Search and Replace |
| `Ctrl+B` | Bold |
| `Ctrl+I` | Italic |
| `Ctrl+K` | Insert Link |
| `F1` | Help |

### Saving Preview HTML

Use `Help > Save Preview HTML` to save the raw HTML generated for the current preview.

This is useful for debugging rendering issues or exporting a lightweight preview document.

### Printing

Use `File > Print` to print the rendered Markdown preview for the current document flavor.

### Troubleshooting

If preview styling appears limited, your system may be using the `wxHtmlWindow` fallback. Installing the wxWidgets WebView development package enables a stronger HTML engine where available.

On Debian or Ubuntu, install:

```text
sudo apt install libwxgtk-webview3.2-dev
```

Then rerun CMake and rebuild the application.

If PlantUML diagrams do not appear in the preview, install `plantuml` so it
is on `PATH`, then restart Glance.

If Mermaid diagrams do not appear in the preview, install Mermaid CLI so
`mmdc` is on `PATH`, then restart Glance.

## Version History

### v1.5.0 (2026-08-28)

- Added preview rendering for `mermaid` and `mmd` fenced code blocks when Mermaid CLI is available
- Added an Insert menu command for creating Mermaid diagram blocks
- Detect PlantUML and Mermaid at runtime on `PATH` so one binary works with or without those tools
- Added two-way synchronization between the editor caret and the rendered preview
- The preview now keeps its position when it re-renders instead of scrolling back to the top

### v1.4.0 (2026-06-12)

- Added preview rendering for `plantuml` and `puml` fenced code blocks when PlantUML is available
- Added an Insert menu command for creating PlantUML diagram blocks
- Renamed the executable and package target to `glance-mde` to avoid a package name conflict

### v1.3.0 (2026-05-21)

- Added Search and Replace functionality
- Added support for editor panel font and font size settings.
- Added special character escaping

### v1.2.0 (2026-05-19)

- Added support for table formatting (left aligned, centered, right aligned columns)
- Added syntax highlighting support
- Added syntax highlighting of code blocks (highlight.js based)

### v1.1.0 (2026-05-15)

- Added Superscript and Subscript support
- Refactored flavor handling to simplify adding new Markdown flavors
- Added help documentation for adding new flavors
- Added function to enable/disable formatting and inserts based on current Markdown flavor

### v1.0.0 (2026-05-15)

- First release
- Full Markdown editing with preview
- Support for GitHub Markdown and Vanilla Markdown flavors
- Tabbed editor with multi-document support
- File tree navigation
- Search and replace functionality
- Comprehensive formatting and insert menus
