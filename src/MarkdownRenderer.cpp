#include "MarkdownRenderer.h"

#include <wx/filename.h>

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {
enum class TableAlignment {
  None,
  Left,
  Center,
  Right,
};

std::string ToStdString(const wxString& value) {
  return std::string(value.utf8_string());
}

wxString ToWxString(const std::string& value) {
  return wxString::FromUTF8(value);
}

std::string EscapeHtml(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size());

  for (char ch : value) {
    switch (ch) {
      case '&':
        escaped += "&amp;";
        break;
      case '<':
        escaped += "&lt;";
        break;
      case '>':
        escaped += "&gt;";
        break;
      case '"':
        escaped += "&quot;";
        break;
      default:
        escaped += ch;
        break;
    }
  }

  return escaped;
}

std::string Trim(const std::string& value) {
  const auto begin =
      std::find_if_not(value.begin(), value.end(),
                       [](unsigned char ch) { return std::isspace(ch); });
  const auto end =
      std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch);
      }).base();

  if (begin >= end) {
    return std::string();
  }

  return std::string(begin, end);
}

bool StartsWith(const std::string& value, const std::string& prefix) {
  return value.rfind(prefix, 0) == 0;
}

std::string ApplyTemplate(
    std::string htmlTemplate,
    const std::vector<std::pair<std::string, std::string>>& replacements) {
  for (const auto& replacement : replacements) {
    size_t position = 0;
    while ((position = htmlTemplate.find(replacement.first, position)) !=
           std::string::npos) {
      htmlTemplate.replace(position, replacement.first.length(),
                           replacement.second);
      position += replacement.second.length();
    }
  }

  return htmlTemplate;
}

bool MatchTag(const MarkdownFlavorDefinition& definition, MarkdownTag tag,
              const std::string& value, std::smatch* match = nullptr) {
  const MarkdownTagDefinition* tagDefinition =
      FindMarkdownTagDefinition(definition, tag);
  if (!tagDefinition) {
    return false;
  }

  const std::regex pattern(tagDefinition->pattern);
  if (match) {
    return std::regex_match(value, *match, pattern);
  }

  return std::regex_match(value, pattern);
}

std::string BuildFencedCodeBlockOpening(const std::smatch& match) {
  if (match.size() < 2 || match[1].str().empty()) {
    return "<pre><code>";
  }

  return "<pre><code class=\"language-" + EscapeHtml(match[1].str()) + "\">";
}

std::vector<std::string> SplitLines(const std::string& text) {
  std::vector<std::string> lines;
  std::stringstream stream(text);
  std::string line;

  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    lines.push_back(line);
  }

  if (!text.empty() && text.back() == '\n') {
    lines.emplace_back();
  }

  return lines;
}

std::vector<std::string> SplitTableRow(const std::string& line) {
  std::string trimmed = Trim(line);
  if (!trimmed.empty() && trimmed.front() == '|') {
    trimmed.erase(trimmed.begin());
  }
  if (!trimmed.empty() && trimmed.back() == '|') {
    trimmed.pop_back();
  }

  std::vector<std::string> cells;
  std::stringstream stream(trimmed);
  std::string cell;
  while (std::getline(stream, cell, '|')) {
    cells.push_back(Trim(cell));
  }

  return cells;
}

bool IsTableSeparator(const std::string& line) {
  const std::vector<std::string> cells = SplitTableRow(line);
  if (cells.empty()) {
    return false;
  }

  return std::all_of(cells.begin(), cells.end(), [](const std::string& cell) {
    std::string trimmed = Trim(cell);
    trimmed.erase(
        std::remove_if(trimmed.begin(), trimmed.end(),
                       [](unsigned char ch) { return std::isspace(ch); }),
        trimmed.end());
    if (trimmed.size() < 3) {
      return false;
    }

    const bool leftColon = trimmed.front() == ':';
    const bool rightColon = trimmed.back() == ':';
    if (leftColon) {
      trimmed.erase(trimmed.begin());
    }
    if (rightColon && !trimmed.empty()) {
      trimmed.pop_back();
    }

    return trimmed.size() >= 3 &&
           std::all_of(trimmed.begin(), trimmed.end(),
                       [](char ch) { return ch == '-'; });
  });
}

std::vector<TableAlignment> ParseTableAlignments(const std::string& line) {
  std::vector<TableAlignment> alignments;
  const std::vector<std::string> cells = SplitTableRow(line);
  alignments.reserve(cells.size());

  for (const std::string& cell : cells) {
    std::string trimmed = Trim(cell);
    trimmed.erase(
        std::remove_if(trimmed.begin(), trimmed.end(),
                       [](unsigned char ch) { return std::isspace(ch); }),
        trimmed.end());

    const bool leftColon = !trimmed.empty() && trimmed.front() == ':';
    const bool rightColon = !trimmed.empty() && trimmed.back() == ':';
    if (leftColon && rightColon) {
      alignments.push_back(TableAlignment::Center);
    } else if (leftColon) {
      alignments.push_back(TableAlignment::Left);
    } else if (rightColon) {
      alignments.push_back(TableAlignment::Right);
    } else {
      alignments.push_back(TableAlignment::None);
    }
  }

  return alignments;
}

std::string TableAlignmentAttribute(TableAlignment alignment) {
  switch (alignment) {
    case TableAlignment::Left:
      return " style=\"text-align: left;\"";
    case TableAlignment::Center:
      return " style=\"text-align: center;\"";
    case TableAlignment::Right:
      return " style=\"text-align: right;\"";
    case TableAlignment::None:
      return "";
  }

  return "";
}

bool IsAbsoluteOrRemotePath(const std::string& path) {
  return path.find("://") != std::string::npos || StartsWith(path, "#") ||
         StartsWith(path, "/");
}

std::string ResolveImagePaths(const std::string& html,
                              const wxString& baseDirectory) {
  if (baseDirectory.empty()) {
    return html;
  }

  static const std::regex imagePattern(
      "<img alt=\"([^\"]*)\" src=\"([^\"]+)\">");
  std::string resolved;
  std::string::const_iterator searchStart = html.begin();
  std::smatch match;

  while (std::regex_search(searchStart, html.cend(), match, imagePattern)) {
    resolved.append(searchStart, match[0].first);

    const std::string alt = match[1].str();
    const std::string source = match[2].str();
    if (IsAbsoluteOrRemotePath(source)) {
      resolved += match[0].str();
    } else {
      wxFileName imagePath(ToWxString(source));
      imagePath.MakeAbsolute(baseDirectory);
      imagePath.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
      resolved += "<img alt=\"" + alt + "\" src=\"file://" +
                  ToStdString(imagePath.GetFullPath()) + "\">";
    }

    searchStart = match[0].second;
  }

  resolved.append(searchStart, html.cend());
  return resolved;
}

std::string ProtectInlineTag(const std::string& html,
                             const MarkdownTagDefinition& tagDefinition,
                             std::vector<std::string>* codeSpans) {
  const std::regex codePattern(tagDefinition.pattern);
  std::string protectedHtml;
  std::string::const_iterator searchStart = html.begin();
  std::smatch match;

  while (std::regex_search(searchStart, html.cend(), match, codePattern)) {
    protectedHtml.append(searchStart, match[0].first);

    const std::string placeholder =
        "\x1f" + std::to_string(codeSpans->size()) + "\x1f";
    codeSpans->push_back(std::regex_replace(match.str(), codePattern,
                                            tagDefinition.htmlTemplate));
    protectedHtml += placeholder;

    searchStart = match[0].second;
  }

  protectedHtml.append(searchStart, html.cend());
  return protectedHtml;
}

std::string ApplyInlineTag(const std::string& html,
                           const MarkdownTagDefinition* tagDefinition) {
  if (!tagDefinition) {
    return html;
  }

  return std::regex_replace(html, std::regex(tagDefinition->pattern),
                            tagDefinition->htmlTemplate);
}

std::string RestoreCodeSpans(std::string html,
                             const std::vector<std::string>& codeSpans) {
  for (size_t i = 0; i < codeSpans.size(); ++i) {
    const std::string placeholder = "\x1f" + std::to_string(i) + "\x1f";
    size_t position = 0;
    while ((position = html.find(placeholder, position)) != std::string::npos) {
      html.replace(position, placeholder.length(), codeSpans[i]);
      position += codeSpans[i].length();
    }
  }

  return html;
}
}  // namespace

wxString MarkdownRenderer::RenderDocument(const wxString& markdown,
                                          const wxString& sourceFilePath,
                                          MarkdownFlavor flavor) const {
  const MarkdownFlavorDefinition& definition =
      GetMarkdownFlavorDefinition(flavor);
  return RenderDocument(markdown, sourceFilePath, definition);
}

wxString MarkdownRenderer::RenderDocument(
    const wxString& markdown, const wxString& sourceFilePath,
    const MarkdownFlavorDefinition& definition) const {
  const std::vector<std::string> lines = SplitLines(ToStdString(markdown));
  wxString baseDirectory;
  if (!sourceFilePath.empty()) {
    baseDirectory = wxFileName(sourceFilePath).GetPath();
  }

  std::string html;
  std::string paragraph;
  bool inCodeBlock = false;
  bool inUnorderedList = false;
  bool inOrderedList = false;
  const MarkdownTagDefinition* fencedCodeBlockRule =
      FindMarkdownTagDefinition(definition, MarkdownTag::FencedCodeBlock);
  const MarkdownTagDefinition* tableRule =
      FindMarkdownTagDefinition(definition, MarkdownTag::Table);
  const MarkdownTagDefinition* unorderedListRule =
      FindMarkdownTagDefinition(definition, MarkdownTag::UnorderedList);
  const MarkdownTagDefinition* orderedListRule =
      FindMarkdownTagDefinition(definition, MarkdownTag::OrderedList);
  const MarkdownTagDefinition* taskListRule =
      FindMarkdownTagDefinition(definition, MarkdownTag::TaskListItem);

  auto closeParagraph = [&]() {
    if (!paragraph.empty()) {
      html += "<p>" +
              ToStdString(RenderInline(ToWxString(paragraph), baseDirectory,
                                       definition)) +
              "</p>\n";
      paragraph.clear();
    }
  };

  auto closeLists = [&]() {
    if (inUnorderedList) {
      html += unorderedListRule ? unorderedListRule->closingHtml : "</ul>\n";
      inUnorderedList = false;
    }
    if (inOrderedList) {
      html += orderedListRule ? orderedListRule->closingHtml : "</ol>\n";
      inOrderedList = false;
    }
  };

  for (size_t i = 0; i < lines.size(); ++i) {
    const std::string line = lines[i];
    const std::string trimmed = Trim(line);
    std::smatch match;

    if (fencedCodeBlockRule &&
        MatchTag(definition, MarkdownTag::FencedCodeBlock, trimmed, &match)) {
      closeParagraph();
      closeLists();
      html += inCodeBlock ? fencedCodeBlockRule->closingHtml
                          : BuildFencedCodeBlockOpening(match);
      inCodeBlock = !inCodeBlock;
      continue;
    }

    if (inCodeBlock) {
      html += EscapeHtml(line) + "\n";
      continue;
    }

    if (trimmed.empty()) {
      closeParagraph();
      closeLists();
      continue;
    }

    if (tableRule && i + 1 < lines.size() &&
        trimmed.find('|') != std::string::npos &&
        IsTableSeparator(lines[i + 1])) {
      closeParagraph();
      closeLists();

      const std::vector<std::string> headers = SplitTableRow(trimmed);
      const std::vector<TableAlignment> alignments =
          ParseTableAlignments(lines[i + 1]);
      html += tableRule->openingHtml;
      for (size_t headerIndex = 0; headerIndex < headers.size();
           ++headerIndex) {
        const TableAlignment alignment = headerIndex < alignments.size()
                                             ? alignments[headerIndex]
                                             : TableAlignment::None;
        html += "<th" + TableAlignmentAttribute(alignment) + ">" +
                ToStdString(RenderInline(ToWxString(headers[headerIndex]),
                                         baseDirectory, definition)) +
                "</th>";
      }
      html += "</tr></thead><tbody>\n";

      i += 2;
      while (i < lines.size() &&
             Trim(lines[i]).find('|') != std::string::npos) {
        html += "<tr>";
        const std::vector<std::string> cells = SplitTableRow(lines[i]);
        for (size_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex) {
          const TableAlignment alignment = cellIndex < alignments.size()
                                               ? alignments[cellIndex]
                                               : TableAlignment::None;
          html += "<td" + TableAlignmentAttribute(alignment) + ">" +
                  ToStdString(RenderInline(ToWxString(cells[cellIndex]),
                                           baseDirectory, definition)) +
                  "</td>";
        }
        html += "</tr>\n";
        ++i;
      }
      --i;
      html += tableRule->closingHtml;
      continue;
    }

    if (MatchTag(definition, MarkdownTag::Heading, trimmed, &match)) {
      closeParagraph();
      closeLists();
      const MarkdownTagDefinition* headingRule =
          FindMarkdownTagDefinition(definition, MarkdownTag::Heading);
      const std::string level = std::to_string(match[1].str().size());
      const std::string content = ToStdString(RenderInline(
          ToWxString(Trim(match[2].str())), baseDirectory, definition));
      html += ApplyTemplate(headingRule->htmlTemplate,
                            {{"$level", level}, {"$content", content}});
      continue;
    }

    if (MatchTag(definition, MarkdownTag::HorizontalRule, trimmed)) {
      closeParagraph();
      closeLists();
      html += FindMarkdownTagDefinition(definition, MarkdownTag::HorizontalRule)
                  ->htmlTemplate;
      continue;
    }

    if (MatchTag(definition, MarkdownTag::Blockquote, trimmed, &match)) {
      closeParagraph();
      closeLists();
      const MarkdownTagDefinition* blockquoteRule =
          FindMarkdownTagDefinition(definition, MarkdownTag::Blockquote);
      const std::string content = ToStdString(
          RenderInline(ToWxString(match[1].str()), baseDirectory, definition));
      html +=
          ApplyTemplate(blockquoteRule->htmlTemplate, {{"$content", content}});
      continue;
    }

    const bool unordered =
        unorderedListRule &&
        MatchTag(definition, MarkdownTag::UnorderedList, line, &match);
    const std::smatch unorderedMatch = match;
    const bool ordered =
        orderedListRule &&
        MatchTag(definition, MarkdownTag::OrderedList, line, &match);
    if (unordered || ordered) {
      closeParagraph();
      if (ordered && !inOrderedList) {
        closeLists();
        html += orderedListRule->openingHtml;
        inOrderedList = true;
      } else if (!ordered && !inUnorderedList) {
        closeLists();
        html += unorderedListRule->openingHtml;
        inUnorderedList = true;
      }

      const std::string itemText =
          ordered ? match[1].str() : unorderedMatch[1].str();
      std::smatch taskMatch;
      if (taskListRule && std::regex_match(itemText, taskMatch,
                                           std::regex(taskListRule->pattern))) {
        const std::string checked =
            Trim(taskMatch[1].str()).empty() ? "" : " checked";
        const std::string content = ToStdString(RenderInline(
            ToWxString(taskMatch[2].str()), baseDirectory, definition));
        html += ApplyTemplate(taskListRule->htmlTemplate,
                              {{"$checked", checked}, {"$content", content}});
      } else {
        const MarkdownTagDefinition* listRule =
            ordered ? orderedListRule : unorderedListRule;
        const std::string content = ToStdString(
            RenderInline(ToWxString(itemText), baseDirectory, definition));
        html += ApplyTemplate(listRule->htmlTemplate, {{"$content", content}});
      }
      continue;
    }

    closeLists();
    if (!paragraph.empty()) {
      paragraph += " ";
    }
    paragraph += trimmed;
  }

  closeParagraph();
  closeLists();
  if (inCodeBlock && fencedCodeBlockRule) {
    html += fencedCodeBlockRule->closingHtml;
  }

  return ToWxString(html);
}

wxString MarkdownRenderer::RenderInline(
    const wxString& text, const wxString& baseDirectory,
    const MarkdownFlavorDefinition& definition) const {
  std::string html = EscapeHtml(ToStdString(text));
  std::vector<std::string> codeSpans;
  if (const MarkdownTagDefinition* inlineCodeRule =
          FindMarkdownTagDefinition(definition, MarkdownTag::InlineCode)) {
    html = ProtectInlineTag(html, *inlineCodeRule, &codeSpans);
  }

  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Image));
  html = ResolveImagePaths(html, baseDirectory);
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Link));
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::BoldItalic));
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Bold));
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Italic));
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Strikethrough));
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Highlight));
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Subscript));
  html = ApplyInlineTag(
      html, FindMarkdownTagDefinition(definition, MarkdownTag::Superscript));
  html = RestoreCodeSpans(std::move(html), codeSpans);

  return ToWxString(html);
}
