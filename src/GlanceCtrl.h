#ifndef GLANCE_CTRL_H
#define GLANCE_CTRL_H

#include <wx/stc/stc.h>

class Document;

enum class MarkdownCommand
{
    Bold,
    Italic,
    BoldItalic,
    Underline,
    Strikethrough,
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
    Date,
    Time,
    DateTime,
    HtmlComment,
    Footnote,
    TocMarker
};

class GlanceCtrl : public wxStyledTextCtrl
{
public:
    GlanceCtrl(wxWindow* parent, Document* document);

    Document* GetDocument() const;
    void LoadFromDocument();
    void SaveToDocument();
    wxString GetEditorStatus() const;
    void ExecuteMarkdownCommand(MarkdownCommand command,
                                const wxString& argument = wxString(),
                                const wxString& secondaryArgument = wxString());

private:
    void ConfigureEditor();
    void WrapSelection(const wxString& prefix, const wxString& suffix, const wxString& placeholder);
    void InsertSnippet(const wxString& snippet);
    void PrefixSelectedLines(const wxString& prefix);
    void NumberSelectedLines();
    void ClearFormatting();

    Document* m_document;
};

#endif // GLANCE_CTRL_H
