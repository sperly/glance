# Codebase Overview

Glance is a wxWidgets desktop Markdown editor. The main window combines a file
tree, a tabbed editor, and a rendered preview.

## Main Flow

1. `GlanceApp` starts the wxWidgets application.
2. `MainFrame` builds menus, split panes, the file tree, editor notebook, and
   preview panel.
3. `FileTreePanel` lets the user select Markdown files from a folder.
4. `EditorNotebook` owns open editor tabs and the `DocumentManager`.
5. `GlanceCtrl` edits one `Document`.
6. `PreviewPanel` asks `MarkdownRenderer` for HTML and displays it.

## Application Shell

`src/GlanceApp.h/cpp`

Creates the application and opens the initial frame. Startup arguments are
handled here before the frame is shown.

`src/MainFrame.h/cpp`

Coordinates the app. It owns the top-level menus, status bar, file tree,
editor notebook, preview panel, recent file/folder menus, and document-level
commands.

Important responsibilities:

- Opening files and folders.
- Saving, printing, and exporting preview HTML.
- Updating preview content when the active document changes.
- Enabling and disabling menu items.
- Choosing and validating a document Markdown flavor.

## Documents

`src/Document.h/cpp`

Represents one Markdown file. It stores the file path, current content,
modified state, last known disk modification time, and selected Markdown
flavor.

`src/DocumentManager.h/cpp`

Owns all open `Document` instances. It normalizes file paths so the same file
is not opened twice, and handles save/close operations for documents.

## Editing

`src/EditorNotebook.h/cpp`

Wraps `wxAuiNotebook` and owns the editor tabs. Each tab is a `GlanceCtrl`.
The notebook posts custom events when editor status, active document, or
document content changes.

It also handles tab closing and save prompts. The tab close path deletes the
page before removing the document from `DocumentManager`, so editor controls
never hold dangling document pointers during close notifications.

`src/GlanceCtrl.h/cpp`

Wraps `wxStyledTextCtrl`. It configures Markdown editing styles and implements
format/insert commands such as bold, lists, fenced code blocks, links, tables,
subscript, and superscript.

## Markdown Flavors

`src/MarkdownFlavor.h/cpp`

Defines built-in Markdown flavors and their tag definitions.

Current built-in flavors:

- `GitHub Markdown`
- `Vanilla Markdown`

A flavor is a list of `MarkdownTagDefinition` values. Those tag definitions
describe supported syntax and HTML output.

`src/DocumentSettingsDialog.h/cpp`

Dialog for selecting the current document Markdown flavor.

`src/MarkdownValidator.h/cpp`

Validates the current document against its selected flavor. It warns when a
document uses syntax that is not available in that flavor.

## Rendering And Preview

`src/MarkdownRenderer.h/cpp`

Converts Markdown text into an HTML fragment. It uses the selected
`MarkdownFlavorDefinition` to decide which tags exist and how they render.

`src/PreviewPanel.h/cpp`

Displays rendered HTML. It uses `wxWebView` when available and falls back to
`wxHtmlWindow` otherwise. Preview updates are debounced to avoid re-rendering
on every keystroke.

## File Tree

`src/FileTreePanel.h/cpp`

Shows Markdown files under an opened folder. It filters supported Markdown
extensions and skips hidden files/folders.

## Dialogs And Resources

`src/HelpDialog.h/cpp`

Shows the embedded `resources/help.md` rendered as HTML.

`src/AboutDialog.h/cpp`

Shows app name, version, and project information.

`src/EmbeddedResources.h`
`src/EmbeddedResources.cpp.in`
`cmake/GlanceResources.cmake`

Embed help text and image resources into the application binary.

`src/ResourcePaths.h/cpp`

Finds resource paths in development and installed builds.

## Settings

`src/SettingsManager.h/cpp`

Uses `wxConfig` to persist:

- Window geometry and splitter positions.
- Recent files and folders.
- Per-document Markdown flavor choices.

## Build System

`CMakeLists.txt`

Defines the application target, source lists, version, wxWidgets dependency,
formatting sources, and test entry point.

`cmake/GlanceWebView.cmake`

Detects and configures wxWebView support.

`cmake/GlanceTests.cmake`

Adds the lightweight core test executable.

`cmake/GlanceResources.cmake`

Generates embedded resource source files.

`cmake/GlancePackaging.cmake`

Configures packaging.

`cmake/GlanceFormat.cmake`

Adds `format` and `format-check` targets for clang-format.

## Tests

`tests/core_tests.cpp`

Contains lightweight tests for document loading/saving, document manager path
handling, Markdown rendering, Markdown flavor behavior, and validation.

Run tests with:

```sh
cmake --build build --target glance_core_tests
ctest --test-dir build --output-on-failure
```
