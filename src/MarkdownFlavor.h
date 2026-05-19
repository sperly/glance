#ifndef MARKDOWN_FLAVOR_H
#define MARKDOWN_FLAVOR_H

#include <wx/arrstr.h>
#include <wx/string.h>

#include <string>
#include <vector>

enum class MarkdownFlavor {
  GitHub,
  Vanilla,
};

enum class MarkdownTagKind {
  Block,
  Inline,
};

enum class MarkdownTag {
  Heading,
  HorizontalRule,
  Blockquote,
  UnorderedList,
  OrderedList,
  TaskListItem,
  FencedCodeBlock,
  Table,
  Image,
  Link,
  BoldItalic,
  Bold,
  Italic,
  InlineCode,
  Highlight,
  Strikethrough,
  Subscript,
  Superscript,
};

struct MarkdownTagDefinition {
  MarkdownTag tag;
  MarkdownTagKind kind;
  wxString displayName;
  std::string pattern;
  std::string htmlTemplate;
  std::string openingHtml;
  std::string closingHtml;
};

struct MarkdownFlavorDefinition {
  MarkdownFlavor flavor;
  wxString id;
  wxString displayName;
  wxString description;
  std::vector<MarkdownTagDefinition> tags;
};

const std::vector<MarkdownFlavorDefinition>& GetMarkdownFlavorDefinitions();
const MarkdownFlavorDefinition& GetMarkdownFlavorDefinition(
    MarkdownFlavor flavor);
const MarkdownTagDefinition* FindMarkdownTagDefinition(
    const MarkdownFlavorDefinition& definition, MarkdownTag tag);
bool MarkdownFlavorHasTag(const MarkdownFlavorDefinition& definition,
                          MarkdownTag tag);
MarkdownFlavor MarkdownFlavorFromId(const wxString& id);
wxString MarkdownFlavorToId(MarkdownFlavor flavor);
wxString MarkdownFlavorToDisplayName(MarkdownFlavor flavor);
wxArrayString GetMarkdownFlavorDisplayNames();
int GetMarkdownFlavorDisplayIndex(MarkdownFlavor flavor);
MarkdownFlavor MarkdownFlavorFromDisplayIndex(int index);

#endif  // MARKDOWN_FLAVOR_H
