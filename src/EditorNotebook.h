#ifndef EDITOR_NOTEBOOK_H
#define EDITOR_NOTEBOOK_H

#include "DocumentManager.h"
#include "GlanceCtrl.h"
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>

wxDECLARE_EVENT(wxEVT_GLANCE_EDITOR_STATUS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GLANCE_ACTIVE_DOCUMENT_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GLANCE_DOCUMENT_CHANGED, wxCommandEvent);

class EditorNotebook : public wxAuiNotebook
{
public:
    explicit EditorNotebook(wxWindow* parent);

    Document* OpenFile(const wxString& filePath, wxString* errorMessage = nullptr);
    bool SaveCurrent(wxString* errorMessage = nullptr);
    bool SaveAll(wxString* errorMessage = nullptr);
    bool CloseCurrent();
    bool ConfirmCloseAll();
    void CloseAllWithoutPrompt();
    bool HasModifiedDocuments() const;
    bool CheckForExternalChanges(wxWindow* parent);

    GlanceCtrl* GetCurrentEditor() const;
    Document* GetCurrentDocument() const;
    wxString GetCurrentContent() const;
    wxString GetCurrentFilePath() const;

    void Undo();
    void Redo();
    void Cut();
    void Copy();
    void Paste();
    void SelectAllText();
    bool ExecuteMarkdownCommand(MarkdownCommand command,
                                const wxString& argument = wxString(),
                                const wxString& secondaryArgument = wxString());

private:
    void OnEditorChanged(wxStyledTextEvent& event);
    void OnEditorUpdateUI(wxStyledTextEvent& event);
    void OnPageChanged(wxAuiNotebookEvent& event);
    void OnPageClose(wxAuiNotebookEvent& event);

    GlanceCtrl* GetEditorAt(size_t pageIndex) const;
    int FindPageForDocument(Document* document) const;
    void UpdatePageTitle(Document* document);
    void SendStatusChanged();
    void SendActiveDocumentChanged();
    void SendDocumentChanged(Document* document);
    bool PromptSaveIfModified(Document* document);
    bool ReloadDocument(Document* document, wxString* errorMessage = nullptr);

    DocumentManager m_documentManager;
    bool m_closingProgrammatically;
};

#endif // EDITOR_NOTEBOOK_H
