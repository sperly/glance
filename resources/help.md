# Glance Help

Glance is a portable Markdown editor with a folder tree, tabbed editor, and rendered preview.

## Opening Folders

Use `File > Open Folder` to choose a folder. Glance scans the folder recursively and shows Markdown files in the left tree.

Supported file types:

- `.md`
- `.markdown`
- `.mdown`
- `.mkd`

Hidden files and folders are skipped by default.

## Opening Files

Open a file by double-clicking it in the file tree, or use `File > Open File`. If the file is already open, Glance switches to the existing tab.

You can also launch Glance with a folder or Markdown file path:

```text
glance /path/to/folder
glance /path/to/file.md
```

## Editing And Saving

Each Markdown file opens in its own tab. Modified files show an asterisk before the tab name.

Use:

- `File > Save File` to save the current tab.
- `File > Save All` to save all modified tabs.
- `File > Close Tab` to close the current tab.

When closing a modified tab, switching folders, or exiting the app, Glance asks whether to save changes.

## Preview

The right pane shows a rendered preview of the active Markdown tab. The preview updates automatically after a short debounce when you type.

The preview supports common Markdown structures:

- Headings
- Paragraphs
- Emphasis and strong text
- Strikethrough
- Lists and task lists
- Code blocks and inline code
- Blockquotes
- Tables
- Links and images
- Horizontal rules

Image paths are resolved relative to the Markdown file location when possible.

## Formatting Commands

Use the `Format` menu to transform selected text or insert placeholders at the caret.

Available commands include:

- Bold
- Italic
- Bold Italic
- Underline
- Strikethrough
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

## Insert Commands

Use the `Insert` menu to add Markdown snippets:

- Link
- Image
- Table
- Code Block
- Inline Code
- Blockquote
- Bullet List
- Numbered List
- Task List
- Horizontal Rule
- Date
- Time
- Date and Time
- HTML Comment
- Footnote
- Table of Contents Marker

The image command prompts for an image file and alt text. Glance inserts a relative path when the image is near the current Markdown file.

## Keyboard Shortcuts

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

## Saving Preview HTML

Use `Help > Save Preview HTML` to save the raw HTML generated for the current preview.

This is useful for debugging rendering issues or exporting a lightweight preview document.

## Printing

Use `File > Print` to print the raw Markdown text. Rendered preview printing can be added later.

## Troubleshooting

If preview styling appears limited, your system may be using the `wxHtmlWindow` fallback. Installing the wxWidgets WebView development package enables a stronger HTML engine where available.

On Debian or Ubuntu, install:

```text
sudo apt install libwxgtk-webview3.2-dev
```

Then rerun CMake and rebuild the application.
