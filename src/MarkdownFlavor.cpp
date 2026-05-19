#include "MarkdownFlavor.h"

#include <algorithm>

namespace {
std::vector<MarkdownTagDefinition> CommonMarkdownTags() {
  return {
      {MarkdownTag::Heading, MarkdownTagKind::Block, "Heading",
       "^(#{1,6})\\s+(.+)$", "<h$level>$content</h$level>\n", "", ""},
      {MarkdownTag::HorizontalRule, MarkdownTagKind::Block, "Horizontal rule",
       "^(?:---|\\*\\*\\*|___)$", "<hr>\n", "", ""},
      {MarkdownTag::Blockquote, MarkdownTagKind::Block, "Blockquote",
       "^>\\s+(.+)$", "<blockquote><p>$content</p></blockquote>\n", "", ""},
      {MarkdownTag::UnorderedList, MarkdownTagKind::Block, "Unordered list",
       "^\\s*[-*+]\\s+(.+)$", "<li>$content</li>\n", "<ul>\n", "</ul>\n"},
      {MarkdownTag::OrderedList, MarkdownTagKind::Block, "Ordered list",
       "^\\s*\\d+\\.\\s+(.+)$", "<li>$content</li>\n", "<ol>\n", "</ol>\n"},
      {MarkdownTag::Image, MarkdownTagKind::Inline, "Image",
       "!\\[([^\\]]*)\\]\\(([^\\)]+)\\)", "<img alt=\"$1\" src=\"$2\">", "",
       ""},
      {MarkdownTag::Link, MarkdownTagKind::Inline, "Link",
       "\\[([^\\]]+)\\]\\(([^\\)]+)\\)", "<a href=\"$2\">$1</a>", "", ""},
      {MarkdownTag::BoldItalic, MarkdownTagKind::Inline, "Bold italic",
       "\\*\\*\\*([^*]+)\\*\\*\\*", "<strong><em>$1</em></strong>", "", ""},
      {MarkdownTag::Bold, MarkdownTagKind::Inline, "Bold",
       "\\*\\*([^*]+)\\*\\*", "<strong>$1</strong>", "", ""},
      {MarkdownTag::Italic, MarkdownTagKind::Inline, "Italic", "\\*([^*]+)\\*",
       "<em>$1</em>", "", ""},
      {MarkdownTag::InlineCode, MarkdownTagKind::Inline, "Inline code",
       "`([^`]+)`", "<code>$1</code>", "", ""},
  };
}

std::vector<MarkdownTagDefinition> GitHubMarkdownTags() {
  std::vector<MarkdownTagDefinition> tags = CommonMarkdownTags();
  tags.push_back({MarkdownTag::TaskListItem, MarkdownTagKind::Block,
                  "Task list item", "^\\[([ xX])\\]\\s+(.+)$",
                  "<li class=\"task\"><input type=\"checkbox\" disabled"
                  "$checked> $content</li>\n",
                  "", ""});
  tags.push_back({MarkdownTag::FencedCodeBlock, MarkdownTagKind::Block,
                  "Fenced code block", "^\\s*```\\s*([A-Za-z0-9_+.-]*)[^`]*$",
                  "<pre><code>$content", "<pre><code>", "</code></pre>\n"});
  tags.push_back({MarkdownTag::Table, MarkdownTagKind::Block, "Table",
                  "^\\s*\\|?.*\\|.*\\n\\s*\\|?\\s*:?-{3,}:?\\s*\\|", "",
                  "<table><thead><tr>", "</tbody></table>\n"});
  tags.push_back({MarkdownTag::Strikethrough, MarkdownTagKind::Inline,
                  "Strikethrough", "~~([^~]+)~~",
                  "<span class=\"glance-strike\">$1</span>", "", ""});
  tags.push_back({MarkdownTag::Highlight, MarkdownTagKind::Inline, "Highlight",
                  "==([^=]+)==", "<mark>$1</mark>", "", ""});
  tags.push_back({MarkdownTag::Subscript, MarkdownTagKind::Inline, "Subscript",
                  "(^|[^~])~([^\\s~]+)~([^~]|$)", "$1<sub>$2</sub>$3", "", ""});
  tags.push_back({MarkdownTag::Superscript, MarkdownTagKind::Inline,
                  "Superscript", "(^|[^\\^])\\^([^\\s\\^]+)\\^([^\\^]|$)",
                  "$1<sup>$2</sup>$3", "", ""});
  return tags;
}

const std::vector<MarkdownFlavorDefinition>& Definitions() {
  static const std::vector<MarkdownFlavorDefinition> definitions = {
      {MarkdownFlavor::GitHub, "github", "GitHub Markdown",
       "GitHub-flavored Markdown with tables, task lists, strikethrough, and "
       "fenced code blocks.",
       GitHubMarkdownTags()},
      {MarkdownFlavor::Vanilla, "vanilla", "Vanilla Markdown",
       "Core Markdown without GitHub-specific extensions.",
       CommonMarkdownTags()},
  };

  return definitions;
}
}  // namespace

const std::vector<MarkdownFlavorDefinition>& GetMarkdownFlavorDefinitions() {
  return Definitions();
}

const MarkdownFlavorDefinition& GetMarkdownFlavorDefinition(
    MarkdownFlavor flavor) {
  const auto& definitions = Definitions();
  const auto it =
      std::find_if(definitions.begin(), definitions.end(),
                   [flavor](const MarkdownFlavorDefinition& definition) {
                     return definition.flavor == flavor;
                   });

  if (it != definitions.end()) {
    return *it;
  }

  return definitions.front();
}

const MarkdownTagDefinition* FindMarkdownTagDefinition(
    const MarkdownFlavorDefinition& definition, MarkdownTag tag) {
  const auto it =
      std::find_if(definition.tags.begin(), definition.tags.end(),
                   [tag](const MarkdownTagDefinition& tagDefinition) {
                     return tagDefinition.tag == tag;
                   });

  return it != definition.tags.end() ? &*it : nullptr;
}

bool MarkdownFlavorHasTag(const MarkdownFlavorDefinition& definition,
                          MarkdownTag tag) {
  return FindMarkdownTagDefinition(definition, tag) != nullptr;
}

MarkdownFlavor MarkdownFlavorFromId(const wxString& id) {
  const auto& definitions = Definitions();
  const auto it =
      std::find_if(definitions.begin(), definitions.end(),
                   [&id](const MarkdownFlavorDefinition& definition) {
                     return definition.id.CmpNoCase(id) == 0;
                   });

  return it != definitions.end() ? it->flavor : MarkdownFlavor::GitHub;
}

wxString MarkdownFlavorToId(MarkdownFlavor flavor) {
  return GetMarkdownFlavorDefinition(flavor).id;
}

wxString MarkdownFlavorToDisplayName(MarkdownFlavor flavor) {
  return GetMarkdownFlavorDefinition(flavor).displayName;
}

wxArrayString GetMarkdownFlavorDisplayNames() {
  wxArrayString names;
  for (const auto& definition : Definitions()) {
    names.Add(definition.displayName);
  }

  return names;
}

int GetMarkdownFlavorDisplayIndex(MarkdownFlavor flavor) {
  const auto& definitions = Definitions();
  for (size_t i = 0; i < definitions.size(); ++i) {
    if (definitions[i].flavor == flavor) {
      return static_cast<int>(i);
    }
  }

  return 0;
}

MarkdownFlavor MarkdownFlavorFromDisplayIndex(int index) {
  const auto& definitions = Definitions();
  if (index < 0 || index >= static_cast<int>(definitions.size())) {
    return MarkdownFlavor::GitHub;
  }

  return definitions[static_cast<size_t>(index)].flavor;
}
