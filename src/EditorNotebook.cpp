#include "EditorNotebook.h"

#include <wx/msgdlg.h>

#include "GlanceCtrl.h"

wxDEFINE_EVENT(wxEVT_GLANCE_EDITOR_STATUS_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_GLANCE_ACTIVE_DOCUMENT_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_GLANCE_DOCUMENT_CHANGED, wxCommandEvent);

EditorNotebook::EditorNotebook(wxWindow* parent)
    : wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                    wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_CLOSE_ON_ACTIVE_TAB),
      m_closingProgrammatically(false) {
  Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &EditorNotebook::OnPageChanged, this);
  Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &EditorNotebook::OnPageClose, this);
}

Document* EditorNotebook::OpenFile(const wxString& filePath,
                                   wxString* errorMessage) {
  Document* document = m_documentManager.OpenDocument(filePath, errorMessage);
  if (!document) {
    return nullptr;
  }

  const int existingPage = FindPageForDocument(document);
  if (existingPage != wxNOT_FOUND) {
    SetSelection(existingPage);
    SendStatusChanged();
    return document;
  }

  GlanceCtrl* editor = new GlanceCtrl(this, document);
  editor->Bind(wxEVT_STC_CHANGE, &EditorNotebook::OnEditorChanged, this);
  editor->Bind(wxEVT_STC_UPDATEUI, &EditorNotebook::OnEditorUpdateUI, this);
  AddPage(editor, document->GetFileName(), true);
  SendActiveDocumentChanged();
  SendStatusChanged();

  return document;
}

bool EditorNotebook::SaveCurrent(wxString* errorMessage) {
  GlanceCtrl* editor = GetCurrentEditor();
  if (!editor) {
    if (errorMessage) {
      *errorMessage = "No document is open.";
    }
    return false;
  }

  editor->SaveToDocument();
  Document* document = editor->GetDocument();
  if (!m_documentManager.SaveDocument(document, errorMessage)) {
    return false;
  }

  editor->SetSavePoint();
  document->SetModified(false);
  UpdatePageTitle(document);
  SendStatusChanged();
  return true;
}

bool EditorNotebook::SaveAll(wxString* errorMessage) {
  for (size_t i = 0; i < GetPageCount(); ++i) {
    if (GlanceCtrl* editor = GetEditorAt(i)) {
      editor->SaveToDocument();
    }
  }

  if (!m_documentManager.SaveAll(errorMessage)) {
    return false;
  }

  for (size_t i = 0; i < GetPageCount(); ++i) {
    if (GlanceCtrl* editor = GetEditorAt(i)) {
      editor->SetSavePoint();
      editor->GetDocument()->SetModified(false);
      UpdatePageTitle(editor->GetDocument());
    }
  }

  SendStatusChanged();
  return true;
}

bool EditorNotebook::CloseCurrent() {
  const int selection = GetSelection();
  if (selection == wxNOT_FOUND) {
    return true;
  }

  GlanceCtrl* editor = GetEditorAt(static_cast<size_t>(selection));
  if (!editor || !PromptSaveIfModified(editor->GetDocument())) {
    return false;
  }

  Document* document = editor->GetDocument();
  m_closingProgrammatically = true;
  DeletePage(static_cast<size_t>(selection));
  m_closingProgrammatically = false;
  m_documentManager.CloseDocument(document);
  SendActiveDocumentChanged();
  SendStatusChanged();
  return true;
}

bool EditorNotebook::ConfirmCloseAll() {
  std::vector<Document*> documentsToCheck;
  for (const auto& document : m_documentManager.GetDocuments()) {
    documentsToCheck.push_back(document.get());
  }

  for (Document* document : documentsToCheck) {
    if (!PromptSaveIfModified(document)) {
      return false;
    }
  }

  return true;
}

void EditorNotebook::CloseAllWithoutPrompt() {
  while (GetPageCount() > 0) {
    GlanceCtrl* editor = GetEditorAt(0);
    Document* document = editor ? editor->GetDocument() : nullptr;
    m_closingProgrammatically = true;
    DeletePage(0);
    m_closingProgrammatically = false;
    if (document) {
      m_documentManager.CloseDocument(document);
    }
  }

  SendActiveDocumentChanged();
  SendStatusChanged();
}

bool EditorNotebook::HasModifiedDocuments() const {
  return m_documentManager.HasModifiedDocuments();
}

bool EditorNotebook::CheckForExternalChanges(wxWindow* parent) {
  bool reloadedAnyDocument = false;
  std::vector<Document*> documentsToCheck;
  for (const auto& document : m_documentManager.GetDocuments()) {
    documentsToCheck.push_back(document.get());
  }

  for (Document* document : documentsToCheck) {
    if (!document || !document->HasChangedOnDisk()) {
      continue;
    }

    wxString message = document->GetFileName() +
                       " changed outside Glance.\n\nReload it from disk?";
    if (document->IsModified()) {
      message += "\n\nReloading will discard your unsaved editor changes.";
    }

    wxMessageDialog dialog(parent ? parent : this, message,
                           "File Changed on Disk", wxYES_NO | wxICON_WARNING);
    const int response = dialog.ShowModal();
    if (response == wxID_YES) {
      wxString errorMessage;
      if (!ReloadDocument(document, &errorMessage)) {
        wxMessageBox(errorMessage, "Reload Failed", wxOK | wxICON_ERROR,
                     parent ? parent : this);
        document->MarkDiskStateCurrent();
        continue;
      }

      reloadedAnyDocument = true;
      continue;
    }

    document->MarkDiskStateCurrent();
  }

  if (reloadedAnyDocument) {
    SendActiveDocumentChanged();
    SendStatusChanged();
  }

  return reloadedAnyDocument;
}

GlanceCtrl* EditorNotebook::GetCurrentEditor() const {
  const int selection = GetSelection();
  if (selection == wxNOT_FOUND) {
    return nullptr;
  }

  return GetEditorAt(static_cast<size_t>(selection));
}

Document* EditorNotebook::GetCurrentDocument() const {
  GlanceCtrl* editor = GetCurrentEditor();
  return editor ? editor->GetDocument() : nullptr;
}

wxString EditorNotebook::GetCurrentContent() const {
  GlanceCtrl* editor = GetCurrentEditor();
  return editor ? editor->GetText() : wxString();
}

wxString EditorNotebook::GetCurrentFilePath() const {
  Document* document = GetCurrentDocument();
  return document ? document->GetFilePath() : wxString();
}

void EditorNotebook::Undo() {
  if (GlanceCtrl* editor = GetCurrentEditor()) editor->Undo();
}

void EditorNotebook::Redo() {
  if (GlanceCtrl* editor = GetCurrentEditor()) editor->Redo();
}

void EditorNotebook::Cut() {
  if (GlanceCtrl* editor = GetCurrentEditor()) editor->Cut();
}

void EditorNotebook::Copy() {
  if (GlanceCtrl* editor = GetCurrentEditor()) editor->Copy();
}

void EditorNotebook::Paste() {
  if (GlanceCtrl* editor = GetCurrentEditor()) editor->Paste();
}

void EditorNotebook::SelectAllText() {
  if (GlanceCtrl* editor = GetCurrentEditor()) editor->SelectAll();
}

bool EditorNotebook::ExecuteMarkdownCommand(MarkdownCommand command,
                                            const wxString& argument,
                                            const wxString& secondaryArgument) {
  GlanceCtrl* editor = GetCurrentEditor();
  if (!editor) {
    return false;
  }

  editor->ExecuteMarkdownCommand(command, argument, secondaryArgument);
  return true;
}

void EditorNotebook::OnEditorChanged(wxStyledTextEvent& event) {
  GlanceCtrl* editor = dynamic_cast<GlanceCtrl*>(event.GetEventObject());
  if (editor && editor->GetDocument()) {
    editor->SaveToDocument();
    UpdatePageTitle(editor->GetDocument());
    SendDocumentChanged(editor->GetDocument());
  }

  SendStatusChanged();
  event.Skip();
}

void EditorNotebook::OnEditorUpdateUI(wxStyledTextEvent& event) {
  SendStatusChanged();
  event.Skip();
}

void EditorNotebook::OnPageChanged(wxAuiNotebookEvent& event) {
  SendActiveDocumentChanged();
  SendStatusChanged();
  event.Skip();
}

void EditorNotebook::OnPageClose(wxAuiNotebookEvent& event) {
  if (m_closingProgrammatically) {
    event.Skip();
    return;
  }

  const int selection = event.GetSelection();
  if (selection == wxNOT_FOUND) {
    event.Veto();
    return;
  }

  GlanceCtrl* editor = GetEditorAt(static_cast<size_t>(selection));
  if (!editor || !PromptSaveIfModified(editor->GetDocument())) {
    event.Veto();
    return;
  }

  Document* document = editor->GetDocument();
  event.Veto();

  m_closingProgrammatically = true;
  DeletePage(static_cast<size_t>(selection));
  m_closingProgrammatically = false;

  m_documentManager.CloseDocument(document);
  SendActiveDocumentChanged();
  SendStatusChanged();
}

GlanceCtrl* EditorNotebook::GetEditorAt(size_t pageIndex) const {
  if (pageIndex >= GetPageCount()) {
    return nullptr;
  }

  return dynamic_cast<GlanceCtrl*>(GetPage(pageIndex));
}

int EditorNotebook::FindPageForDocument(Document* document) const {
  for (size_t i = 0; i < GetPageCount(); ++i) {
    GlanceCtrl* editor = GetEditorAt(i);
    if (editor && editor->GetDocument() == document) {
      return static_cast<int>(i);
    }
  }

  return wxNOT_FOUND;
}

void EditorNotebook::UpdatePageTitle(Document* document) {
  const int page = FindPageForDocument(document);
  if (page == wxNOT_FOUND || !document) {
    return;
  }

  wxString title = document->GetFileName();
  if (document->IsModified()) {
    title.Prepend("*");
  }

  SetPageText(static_cast<size_t>(page), title);
}

void EditorNotebook::SendStatusChanged() {
  wxCommandEvent event(wxEVT_GLANCE_EDITOR_STATUS_CHANGED, GetId());
  event.SetEventObject(this);

  if (GlanceCtrl* editor = GetCurrentEditor()) {
    event.SetString(editor->GetEditorStatus());
  } else {
    event.SetString("No document");
  }

  wxPostEvent(this, event);
}

void EditorNotebook::SendActiveDocumentChanged() {
  wxCommandEvent event(wxEVT_GLANCE_ACTIVE_DOCUMENT_CHANGED, GetId());
  event.SetEventObject(this);

  if (Document* document = GetCurrentDocument()) {
    event.SetString(document->GetFilePath());
  }

  wxPostEvent(this, event);
}

void EditorNotebook::SendDocumentChanged(Document* document) {
  wxCommandEvent event(wxEVT_GLANCE_DOCUMENT_CHANGED, GetId());
  event.SetEventObject(this);
  if (document) {
    event.SetString(document->GetFilePath());
  }

  wxPostEvent(this, event);
}

bool EditorNotebook::PromptSaveIfModified(Document* document) {
  if (!document || !document->IsModified()) {
    return true;
  }

  wxMessageDialog dialog(
      this, "Save changes to " + document->GetFileName() + "?",
      "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_WARNING);
  const int response = dialog.ShowModal();
  if (response == wxID_CANCEL) {
    return false;
  }
  if (response == wxID_NO) {
    return true;
  }

  wxString errorMessage;
  const int page = FindPageForDocument(document);
  if (page != wxNOT_FOUND) {
    if (GlanceCtrl* editor = GetEditorAt(static_cast<size_t>(page))) {
      editor->SaveToDocument();
    }
  }

  if (!m_documentManager.SaveDocument(document, &errorMessage)) {
    wxMessageBox(errorMessage, "Save Failed", wxOK | wxICON_ERROR, this);
    return false;
  }

  document->SetModified(false);
  UpdatePageTitle(document);
  return true;
}

bool EditorNotebook::ReloadDocument(Document* document,
                                    wxString* errorMessage) {
  const int page = FindPageForDocument(document);
  if (page == wxNOT_FOUND || !document) {
    if (errorMessage) {
      *errorMessage = "The document is no longer open.";
    }
    return false;
  }

  if (!document->Load(errorMessage)) {
    return false;
  }

  if (GlanceCtrl* editor = GetEditorAt(static_cast<size_t>(page))) {
    editor->LoadFromDocument();
    editor->SetSavePoint();
  }

  UpdatePageTitle(document);
  SendDocumentChanged(document);
  return true;
}
