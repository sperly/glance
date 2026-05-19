#include "MarkdownValidator.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::string ToStdString(const wxString& value) {
  return std::string(value.utf8_string());
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

bool MatchesTag(const MarkdownFlavorDefinition& definition, MarkdownTag tag,
                const std::string& value) {
  const MarkdownTagDefinition* tagDefinition =
      FindMarkdownTagDefinition(definition, tag);
  return tagDefinition &&
         std::regex_match(value, std::regex(tagDefinition->pattern));
}

bool ContainsTag(const MarkdownFlavorDefinition& definition, MarkdownTag tag,
                 const std::string& value) {
  const MarkdownTagDefinition* tagDefinition =
      FindMarkdownTagDefinition(definition, tag);
  return tagDefinition &&
         std::regex_search(value, std::regex(tagDefinition->pattern));
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
    const std::string trimmed = Trim(cell);
    if (trimmed.size() < 3) {
      return false;
    }

    return std::all_of(trimmed.begin(), trimmed.end(), [](char ch) {
      return ch == '-' || ch == ':' ||
             std::isspace(static_cast<unsigned char>(ch));
    });
  });
}

bool IsListItem(const std::string& line) {
  static const std::regex listPattern("^\\s*(?:[-*+]|\\d+\\.)\\s+.+");
  return std::regex_match(line, listPattern);
}

std::string StripListMarker(const std::string& line) {
  static const std::regex listMarker("^\\s*(?:[-*+]|\\d+\\.)\\s+");
  return std::regex_replace(line, listMarker, "");
}

bool IsTaskItem(const MarkdownFlavorDefinition& definition,
                const std::string& line) {
  if (!IsListItem(line)) {
    return false;
  }

  const std::string itemText = StripListMarker(line);
  return MatchesTag(definition, MarkdownTag::TaskListItem, itemText);
}

void AddDiagnostic(std::vector<MarkdownDiagnostic>* diagnostics, size_t line,
                   MarkdownDiagnosticSeverity severity,
                   const wxString& message) {
  diagnostics->push_back({line, severity, message});
}
}  // namespace

std::vector<MarkdownDiagnostic> MarkdownValidator::Validate(
    const wxString& markdown, MarkdownFlavor flavor) const {
  const MarkdownFlavorDefinition& definition =
      GetMarkdownFlavorDefinition(flavor);
  return Validate(markdown, definition);
}

std::vector<MarkdownDiagnostic> MarkdownValidator::Validate(
    const wxString& markdown,
    const MarkdownFlavorDefinition& definition) const {
  const MarkdownFlavorDefinition& referenceDefinition =
      GetMarkdownFlavorDefinition(MarkdownFlavor::GitHub);
  const std::vector<std::string> lines = SplitLines(ToStdString(markdown));
  std::vector<MarkdownDiagnostic> diagnostics;
  bool inFencedCodeBlock = false;
  size_t openingFenceLine = 0;

  auto missingTag = [&](MarkdownTag tag) {
    return MarkdownFlavorHasTag(referenceDefinition, tag) &&
           !MarkdownFlavorHasTag(definition, tag);
  };

  auto unsupportedMessage = [&](MarkdownTag tag) {
    const MarkdownTagDefinition* tagDefinition =
        FindMarkdownTagDefinition(referenceDefinition, tag);
    return (tagDefinition ? tagDefinition->displayName : "This syntax") +
           " is not supported by " + definition.displayName + ".";
  };

  for (size_t i = 0; i < lines.size(); ++i) {
    const size_t lineNumber = i + 1;
    const std::string& line = lines[i];
    const std::string trimmed = Trim(line);

    if (MatchesTag(referenceDefinition, MarkdownTag::FencedCodeBlock,
                   trimmed)) {
      if (missingTag(MarkdownTag::FencedCodeBlock)) {
        AddDiagnostic(&diagnostics, lineNumber,
                      MarkdownDiagnosticSeverity::Warning,
                      unsupportedMessage(MarkdownTag::FencedCodeBlock));
        continue;
      }

      if (!inFencedCodeBlock) {
        openingFenceLine = lineNumber;
      }
      inFencedCodeBlock = !inFencedCodeBlock;
      continue;
    }

    if (inFencedCodeBlock) {
      continue;
    }

    if (missingTag(MarkdownTag::Table) && i + 1 < lines.size() &&
        trimmed.find('|') != std::string::npos &&
        IsTableSeparator(lines[i + 1])) {
      AddDiagnostic(&diagnostics, lineNumber,
                    MarkdownDiagnosticSeverity::Warning,
                    unsupportedMessage(MarkdownTag::Table));
    }

    if (missingTag(MarkdownTag::TaskListItem) &&
        IsTaskItem(referenceDefinition, line)) {
      AddDiagnostic(&diagnostics, lineNumber,
                    MarkdownDiagnosticSeverity::Warning,
                    unsupportedMessage(MarkdownTag::TaskListItem));
    }

    if (missingTag(MarkdownTag::Strikethrough) &&
        ContainsTag(referenceDefinition, MarkdownTag::Strikethrough, line)) {
      AddDiagnostic(&diagnostics, lineNumber,
                    MarkdownDiagnosticSeverity::Warning,
                    unsupportedMessage(MarkdownTag::Strikethrough));
    }

    if (missingTag(MarkdownTag::Highlight) &&
        ContainsTag(referenceDefinition, MarkdownTag::Highlight, line)) {
      AddDiagnostic(&diagnostics, lineNumber,
                    MarkdownDiagnosticSeverity::Warning,
                    unsupportedMessage(MarkdownTag::Highlight));
    }

    if (missingTag(MarkdownTag::Subscript) &&
        ContainsTag(referenceDefinition, MarkdownTag::Subscript, line)) {
      AddDiagnostic(&diagnostics, lineNumber,
                    MarkdownDiagnosticSeverity::Warning,
                    unsupportedMessage(MarkdownTag::Subscript));
    }

    if (missingTag(MarkdownTag::Superscript) &&
        ContainsTag(referenceDefinition, MarkdownTag::Superscript, line)) {
      AddDiagnostic(&diagnostics, lineNumber,
                    MarkdownDiagnosticSeverity::Warning,
                    unsupportedMessage(MarkdownTag::Superscript));
    }
  }

  if (inFencedCodeBlock) {
    AddDiagnostic(&diagnostics, openingFenceLine,
                  MarkdownDiagnosticSeverity::Error,
                  "Fenced code block is missing its closing fence.");
  }

  return diagnostics;
}
