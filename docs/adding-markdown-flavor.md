# Adding A Markdown Flavor

Glance represents a Markdown flavor as a named collection of tag definitions.
Each tag definition tells the renderer, validator, and command gating code what
syntax exists for that flavor and how that syntax should become HTML.

The main files are:

- `src/MarkdownFlavor.h`
- `src/MarkdownFlavor.cpp`
- `src/MarkdownRenderer.cpp`
- `src/MarkdownValidator.cpp`
- `src/MainFrame.cpp`
- `src/GlanceCtrl.cpp`

## Flavor Model

`MarkdownFlavorDefinition` contains:

- `flavor`: enum value used for persistence and selection.
- `id`: stable string saved in settings.
- `displayName`: shown in the document settings dialog.
- `description`: shown in the document settings dialog.
- `tags`: the Markdown syntax available in this flavor.

`MarkdownTagDefinition` contains:

- `tag`: semantic tag id, such as `Bold`, `Table`, or `FencedCodeBlock`.
- `kind`: `Inline` or `Block`.
- `displayName`: shown in validation messages.
- `pattern`: regular expression used to recognize the syntax.
- `htmlTemplate`: replacement template for inline tags and simple block tags.
- `openingHtml`: wrapper used by multi-line block tags.
- `closingHtml`: wrapper used by multi-line block tags.

## Adding A Built-In Flavor

1. Add a value to `MarkdownFlavor` in `src/MarkdownFlavor.h`.
2. Add a helper function in `src/MarkdownFlavor.cpp` that returns a
   `std::vector<MarkdownTagDefinition>`.
3. Add a `MarkdownFlavorDefinition` entry to `Definitions()`.
4. Add or update tests in `tests/core_tests.cpp`.

Example:

```cpp
std::vector<MarkdownTagDefinition> MyMarkdownTags() {
  std::vector<MarkdownTagDefinition> tags = CommonMarkdownTags();
  tags.push_back({MarkdownTag::Strikethrough, MarkdownTagKind::Inline,
                  "Strikethrough", "~~([^~]+)~~",
                  "<span class=\"glance-strike\">$1</span>", "", ""});
  return tags;
}
```

Then register it:

```cpp
{MarkdownFlavor::MyFlavor, "my-flavor", "My Flavor",
 "A custom Markdown flavor.", MyMarkdownTags()},
```

## Inline Tags

Inline tags are applied in `MarkdownRenderer::RenderInline()`. A basic inline
tag uses a regex pattern and an HTML replacement template.

Examples:

```cpp
{MarkdownTag::Bold, MarkdownTagKind::Inline, "Bold",
 "\\*\\*([^*]+)\\*\\*", "<strong>$1</strong>", "", ""}

{MarkdownTag::Superscript, MarkdownTagKind::Inline, "Superscript",
 "(^|[^\\^])\\^([^\\s\\^]+)\\^([^\\^]|$)",
 "$1<sup>$2</sup>$3", "", ""}
```

Inline code is protected before other inline formatting runs, so formatting
inside code spans does not get transformed.

If you add a new inline `MarkdownTag`, also add it to
`MarkdownRenderer::RenderInline()` in the order it should be applied.

## Block Tags

Simple block tags, such as headings, horizontal rules, and blockquotes, use a
pattern and an HTML template.

Multi-line structural tags, such as lists, fenced code blocks, and tables, use
special parser support in `MarkdownRenderer.cpp`. Their definitions still live
in the flavor because the flavor controls whether the tag exists and what HTML
wrappers it emits.

Examples:

```cpp
{MarkdownTag::FencedCodeBlock, MarkdownTagKind::Block,
 "Fenced code block", "^\\s*```.*$", "<pre><code>$content",
 "<pre><code>", "</code></pre>\n"}

{MarkdownTag::Table, MarkdownTagKind::Block, "Table",
 "^\\s*\\|?.*\\|.*\\n\\s*\\|?\\s*:?-{3,}:?\\s*\\|",
 "", "<table><thead><tr>", "</tbody></table>\n"}
```

If a new block tag needs unique parsing behavior, add parser support in
`MarkdownRenderer.cpp` and validation support in `MarkdownValidator.cpp`.

## Command Gating

Format and insert menu items are enabled only when the current document flavor
contains the tag required by that command.

The mapping lives in `GetRequiredTagForMarkdownCommand()` in
`src/MainFrame.cpp`.

When adding a command:

1. Add the command to `MarkdownCommand` in `src/GlanceCtrl.h`.
2. Implement the inserted syntax in `GlanceCtrl::ExecuteMarkdownCommand()`.
3. Add a menu item in `MainFrame::CreateMenuBar()`.
4. Add the menu id to `GetMarkdownMenuCommands()`.
5. Map the command to a `MarkdownTag` in
   `GetRequiredTagForMarkdownCommand()`.

## Validation

`MarkdownValidator` compares the selected flavor against the GitHub Markdown
reference flavor. If the document uses GitHub syntax that is missing from the
selected flavor, validation reports a warning.

When adding syntax that should be validated:

- Add a `MarkdownTagDefinition` with a useful `displayName`.
- Add missing-tag detection in `MarkdownValidator.cpp` if the syntax is not
  already covered.
- Add tests in `tests/core_tests.cpp`.

## Testing Checklist

After adding a flavor or tag, run:

```sh
cmake --build build --target glance_core_tests
ctest --test-dir build --output-on-failure
cmake --build build --target glance
```

If `clang-format` is installed, also run:

```sh
cmake --build build --target format-check
```
