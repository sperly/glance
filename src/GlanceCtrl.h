#ifndef GLANCE_CTRL_H
#define GLANCE_CTRL_H

#include <wx/font.h>
#include <wx/stc/stc.h>

class Document;

enum class MarkdownCommand {
  Bold,
  Italic,
  BoldItalic,
  Underline,
  Strikethrough,
  Highlight,
  Subscript,
  Superscript,
  InlineCode,
  CodeBlock,
  Blockquote,
  Heading1,
  Heading2,
  Heading3,
  Heading4,
  Heading5,
  Heading6,
  BulletList,
  NumberedList,
  TaskList,
  CompletedTask,
  HorizontalRule,
  ClearFormatting,
  Link,
  Image,
  Table,
  PlantUmlDiagram,
  MermaidDiagram,
  Date,
  Time,
  DateTime,
  HtmlComment,
  Footnote,
  TocMarker
};

class GlanceCtrl : public wxStyledTextCtrl {
 public:
  GlanceCtrl(wxWindow* parent, Document* document, const wxFont& editorFont);

  Document* GetDocument() const;
  void LoadFromDocument();
  void SaveToDocument();
  void SetEditorFont(const wxFont& font);
  wxString GetEditorStatus() const;
  int GetCurrentSourceLine() const;
  void GotoSourceLine(int sourceLine);
  bool FindNextText(const wxString& searchText, bool matchCase, bool wrap);
  bool ReplaceNextText(const wxString& searchText,
                       const wxString& replacementText, bool matchCase,
                       bool wrap);
  int ReplaceAllText(const wxString& searchText,
                     const wxString& replacementText, bool matchCase);
  void ExecuteMarkdownCommand(MarkdownCommand command,
                              const wxString& argument = wxString(),
                              const wxString& secondaryArgument = wxString());

 private:
  void ConfigureEditor();
  void WrapSelection(const wxString& prefix, const wxString& suffix,
                     const wxString& placeholder);
  void InsertSnippet(const wxString& snippet);
  void PrefixSelectedLines(const wxString& prefix);
  void NumberSelectedLines();
  void ClearFormatting();

  Document* m_document;
  wxFont m_editorFont;
};

#endif  // GLANCE_CTRL_H
