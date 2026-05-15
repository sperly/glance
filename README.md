# Glance Markdown Editor
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

When closing a modified tab, switching folders, or exiting the app, Glance asks whether to save changes.
Document editing, insertion, saving, closing, preview export, and printing commands are disabled when no document is open.

### Document Markdown Flavor

Use `Document > Settings` to choose the Markdown flavor for the current document. Glance uses `GitHub Markdown` by default and also supports `Vanilla Markdown`.

The selected flavor controls preview rendering, printing, preview HTML export, validation, and which Markdown formatting or insertion commands are available. Commands that do not apply to the current flavor are disabled.

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
- Blockquotes
- Tables
- Links and images
- Horizontal rules

Vanilla Markdown mode supports the core Markdown subset without GitHub-specific extensions such as tables, task lists, strikethrough, subscript, and superscript.

Image paths are resolved relative to the Markdown file location when possible.

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
- Bullet List
- Numbered List
- Task List
- Horizontal Rule
- Date
- Time
- Date and Time

The link command prompts for link text and URL. The image command prompts for an image file and alt text. Glance inserts a relative path when the image is near the current Markdown file. The table command asks how many columns to create.

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
