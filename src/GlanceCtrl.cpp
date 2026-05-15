#include "GlanceCtrl.h"

#include <wx/datetime.h>
#include <wx/font.h>

#include <algorithm>
#include <cstdlib>

#include "Document.h"

GlanceCtrl::GlanceCtrl(wxWindow* parent, Document* document)
    : wxStyledTextCtrl(parent, wxID_ANY), m_document(document) {
  ConfigureEditor();
  LoadFromDocument();
}

Document* GlanceCtrl::GetDocument() const { return m_document; }

void GlanceCtrl::LoadFromDocument() {
  SetText(m_document ? m_document->GetContent() : wxString());
  EmptyUndoBuffer();
  SetSavePoint();
}

void GlanceCtrl::SaveToDocument() {
  if (m_document) {
    m_document->SetContent(GetText());
    m_document->SetModified(GetModify());
  }
}

wxString GlanceCtrl::GetEditorStatus() const {
  const int position = GetCurrentPos();
  const int line = LineFromPosition(position) + 1;
  const int column = GetColumn(position) + 1;
  const int selectionLength = std::abs(GetSelectionEnd() - GetSelectionStart());

  return wxString::Format("Line %d, Column %d | Selection %d | Length %d", line,
                          column, selectionLength, GetTextLength());
}

void GlanceCtrl::ExecuteMarkdownCommand(MarkdownCommand command,
                                        const wxString& argument,
                                        const wxString& secondaryArgument) {
  switch (command) {
    case MarkdownCommand::Bold:
      WrapSelection("**", "**", "bold text");
      break;
    case MarkdownCommand::Italic:
      WrapSelection("*", "*", "italic text");
      break;
    case MarkdownCommand::BoldItalic:
      WrapSelection("***", "***", "bold italic text");
      break;
    case MarkdownCommand::Underline:
      WrapSelection("<u>", "</u>", "underlined text");
      break;
    case MarkdownCommand::Strikethrough:
      WrapSelection("~~", "~~", "struck text");
      break;
    case MarkdownCommand::Subscript:
      WrapSelection("~", "~", "lower text");
      break;
    case MarkdownCommand::Superscript:
      WrapSelection("^", "^", "upper text");
      break;
    case MarkdownCommand::InlineCode:
      WrapSelection("`", "`", "code");
      break;
    case MarkdownCommand::CodeBlock:
      InsertSnippet("\n```\ncode block\n```\n");
      break;
    case MarkdownCommand::Blockquote:
      PrefixSelectedLines("> ");
      break;
    case MarkdownCommand::Heading1:
      PrefixSelectedLines("# ");
      break;
    case MarkdownCommand::Heading2:
      PrefixSelectedLines("## ");
      break;
    case MarkdownCommand::Heading3:
      PrefixSelectedLines("### ");
      break;
    case MarkdownCommand::Heading4:
      PrefixSelectedLines("#### ");
      break;
    case MarkdownCommand::Heading5:
      PrefixSelectedLines("##### ");
      break;
    case MarkdownCommand::Heading6:
      PrefixSelectedLines("###### ");
      break;
    case MarkdownCommand::BulletList:
      PrefixSelectedLines("- ");
      break;
    case MarkdownCommand::NumberedList:
      NumberSelectedLines();
      break;
    case MarkdownCommand::TaskList:
      PrefixSelectedLines("- [ ] ");
      break;
    case MarkdownCommand::CompletedTask:
      PrefixSelectedLines("- [x] ");
      break;
    case MarkdownCommand::HorizontalRule:
      InsertSnippet("\n---\n");
      break;
    case MarkdownCommand::ClearFormatting:
      ClearFormatting();
      break;
    case MarkdownCommand::Link:
      InsertSnippet(wxString::Format(
          "[%s](%s)",
          secondaryArgument.empty() ? "link text" : secondaryArgument,
          argument.empty() ? "https://example.com" : argument));
      break;
    case MarkdownCommand::Image:
      InsertSnippet(wxString::Format(
          "![%s](%s)",
          secondaryArgument.empty() ? "alt text" : secondaryArgument,
          argument.empty() ? "image.png" : argument));
      break;
    case MarkdownCommand::Table: {
      int columns = wxAtoi(argument);
      if (columns < 1) {
        columns = 3;
      }
      columns = std::clamp(columns, 1, 12);

      wxString header = "\n|";
      wxString separator = "|";
      wxString row = "|";
      for (int column = 1; column <= columns; ++column) {
        header += wxString::Format(" Column %d |", column);
        separator += "---|";
        row += wxString::Format(" Value %d |", column);
      }

      InsertSnippet(header + "\n" + separator + "\n" + row + "\n");
      break;
    }
    case MarkdownCommand::Date:
      InsertSnippet(wxDateTime::Now().FormatISODate());
      break;
    case MarkdownCommand::Time:
      InsertSnippet(wxDateTime::Now().FormatISOTime());
      break;
    case MarkdownCommand::DateTime:
      InsertSnippet(wxDateTime::Now().FormatISOCombined(' '));
      break;
    case MarkdownCommand::HtmlComment:
      InsertSnippet("<!-- comment -->");
      break;
    case MarkdownCommand::Footnote:
      InsertSnippet("[^1]\n\n[^1]: Footnote text\n");
      break;
    case MarkdownCommand::TocMarker:
      InsertSnippet("[TOC]");
      break;
  }
}

void GlanceCtrl::ConfigureEditor() {
  SetLexer(wxSTC_LEX_MARKDOWN);
  StyleClearAll();
  StyleSetFont(wxSTC_STYLE_DEFAULT,
               wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));
  StyleSetForeground(wxSTC_MARKDOWN_HEADER1, wxColour(25, 80, 150));
  StyleSetForeground(wxSTC_MARKDOWN_HEADER2, wxColour(25, 80, 150));
  StyleSetForeground(wxSTC_MARKDOWN_HEADER3, wxColour(25, 80, 150));
  StyleSetForeground(wxSTC_MARKDOWN_STRONG1, wxColour(120, 60, 20));
  StyleSetBold(wxSTC_MARKDOWN_STRONG1, true);
  StyleSetForeground(wxSTC_MARKDOWN_EM1, wxColour(120, 60, 20));
  StyleSetItalic(wxSTC_MARKDOWN_EM1, true);
  StyleSetForeground(wxSTC_MARKDOWN_CODE, wxColour(40, 110, 80));
  StyleSetForeground(wxSTC_MARKDOWN_CODE2, wxColour(40, 110, 80));
  StyleSetForeground(wxSTC_MARKDOWN_CODEBK, wxColour(40, 110, 80));

  SetMarginType(0, wxSTC_MARGIN_NUMBER);
  SetMarginWidth(0, 48);
  SetTabWidth(4);
  SetUseTabs(false);
  SetWrapMode(wxSTC_WRAP_WORD);
  SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  SetCaretLineVisible(true);
  SetCaretLineBackground(wxColour(245, 248, 252));
}

void GlanceCtrl::WrapSelection(const wxString& prefix, const wxString& suffix,
                               const wxString& placeholder) {
  const int selectionStart = GetSelectionStart();
  const int selectionEnd = GetSelectionEnd();
  const bool hasSelection = selectionStart != selectionEnd;
  const wxString text = hasSelection ? GetSelectedText() : placeholder;
  const wxString replacement = prefix + text + suffix;

  BeginUndoAction();
  ReplaceSelection(replacement);
  EndUndoAction();

  if (!hasSelection) {
    const int placeholderStart =
        selectionStart + static_cast<int>(prefix.length());
    SetSelection(placeholderStart,
                 placeholderStart + static_cast<int>(placeholder.length()));
  } else {
    SetSelection(selectionStart,
                 selectionStart + static_cast<int>(replacement.length()));
  }
}

void GlanceCtrl::InsertSnippet(const wxString& snippet) {
  int selectionStart = GetSelectionStart();
  int selectionEnd = GetSelectionEnd();
  if (selectionEnd < selectionStart) {
    std::swap(selectionStart, selectionEnd);
  }

  BeginUndoAction();
  InsertText(selectionStart, snippet);
  EndUndoAction();

  const int insertedLength = static_cast<int>(snippet.length());
  if (selectionStart != selectionEnd) {
    SetSelection(selectionStart + insertedLength,
                 selectionEnd + insertedLength);
    return;
  }

  SetSelection(selectionStart + insertedLength,
               selectionStart + insertedLength);
}

void GlanceCtrl::PrefixSelectedLines(const wxString& prefix) {
  int selectionStart = GetSelectionStart();
  int selectionEnd = GetSelectionEnd();
  if (selectionEnd < selectionStart) {
    std::swap(selectionStart, selectionEnd);
  }

  int startLine = LineFromPosition(selectionStart);
  int endLine = LineFromPosition(selectionEnd);
  if (selectionEnd > selectionStart &&
      PositionFromLine(endLine) == selectionEnd) {
    --endLine;
  }

  BeginUndoAction();
  for (int line = startLine; line <= endLine; ++line) {
    InsertText(PositionFromLine(line), prefix);
  }
  EndUndoAction();
}

void GlanceCtrl::NumberSelectedLines() {
  int selectionStart = GetSelectionStart();
  int selectionEnd = GetSelectionEnd();
  if (selectionEnd < selectionStart) {
    std::swap(selectionStart, selectionEnd);
  }

  int startLine = LineFromPosition(selectionStart);
  int endLine = LineFromPosition(selectionEnd);
  if (selectionEnd > selectionStart &&
      PositionFromLine(endLine) == selectionEnd) {
    --endLine;
  }

  BeginUndoAction();
  for (int line = startLine; line <= endLine; ++line) {
    const wxString prefix = wxString::Format("%d. ", line - startLine + 1);
    InsertText(PositionFromLine(line), prefix);
  }
  EndUndoAction();
}

void GlanceCtrl::ClearFormatting() {
  int selectionStart = GetSelectionStart();
  int selectionEnd = GetSelectionEnd();
  const bool hasSelection = selectionStart != selectionEnd;

  if (!hasSelection) {
    const int line = GetCurrentLine();
    selectionStart = PositionFromLine(line);
    selectionEnd = GetLineEndPosition(line);
    SetSelection(selectionStart, selectionEnd);
  }

  wxString text = GetSelectedText();
  text.Replace("***", "");
  text.Replace("**", "");
  text.Replace("*", "");
  text.Replace("~~", "");
  text.Replace("`", "");
  text.Replace("<u>", "");
  text.Replace("</u>", "");

  const wxArrayString markers = {"# ",      "## ", "### ",   "#### ",  "##### ",
                                 "###### ", "> ",  "- [ ] ", "- [x] ", "- "};

  wxArrayString lines = wxSplit(text, '\n', '\0');
  for (wxString& line : lines) {
    for (const wxString& marker : markers) {
      if (line.StartsWith(marker)) {
        line = line.Mid(marker.length());
        break;
      }
    }
  }

  text = wxJoin(lines, '\n');
  BeginUndoAction();
  ReplaceSelection(text);
  EndUndoAction();
  SetSelection(selectionStart,
               selectionStart + static_cast<int>(text.length()));
}
