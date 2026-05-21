#ifndef EDITOR_NOTEBOOK_H
#define EDITOR_NOTEBOOK_H

#include <wx/aui/aui.h>
#include <wx/font.h>
#include <wx/stc/stc.h>

#include "DocumentManager.h"
#include "GlanceCtrl.h"

wxDECLARE_EVENT(wxEVT_GLANCE_EDITOR_STATUS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GLANCE_ACTIVE_DOCUMENT_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GLANCE_DOCUMENT_CHANGED, wxCommandEvent);

class EditorNotebook : public wxAuiNotebook {
 public:
  explicit EditorNotebook(wxWindow* parent);

  Document* OpenFile(const wxString& filePath,
                     wxString* errorMessage = nullptr);
  bool SaveCurrent(wxString* errorMessage = nullptr);
  bool SaveAll(wxString* errorMessage = nullptr);
  bool CloseCurrent();
  bool ConfirmCloseAll();
  void CloseAllWithoutPrompt();
  bool HasModifiedDocuments() const;
  bool CheckForExternalChanges(wxWindow* parent);
  void SetEditorFont(const wxFont& font);
  wxFont GetEditorFont() const;

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
  bool FindNextText(const wxString& searchText, bool matchCase, bool wrap);
  bool ReplaceNextText(const wxString& searchText,
                       const wxString& replacementText, bool matchCase,
                       bool wrap);
  int ReplaceAllText(const wxString& searchText,
                     const wxString& replacementText, bool matchCase);
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
  wxFont m_editorFont;
};

#endif  // EDITOR_NOTEBOOK_H
