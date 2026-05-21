#ifndef SEARCH_REPLACE_DIALOG_H
#define SEARCH_REPLACE_DIALOG_H

#include <wx/dialog.h>
#include <wx/string.h>

class EditorNotebook;
class wxCheckBox;
class wxStaticText;
class wxTextCtrl;

class SearchReplaceDialog : public wxDialog {
 public:
  SearchReplaceDialog(wxWindow* parent, EditorNotebook* editorNotebook);

 private:
  void OnFindNext(wxCommandEvent& event);
  void OnReplace(wxCommandEvent& event);
  void OnReplaceAll(wxCommandEvent& event);
  void OnClose(wxCommandEvent& event);
  void SetStatus(const wxString& message);
  wxString GetSearchText() const;
  wxString GetReplacementText() const;
  bool MatchCase() const;
  bool WrapSearch() const;

  EditorNotebook* m_editorNotebook;
  wxTextCtrl* m_findText;
  wxTextCtrl* m_replaceText;
  wxCheckBox* m_matchCase;
  wxCheckBox* m_wrapSearch;
  wxStaticText* m_statusLabel;
};

#endif  // SEARCH_REPLACE_DIALOG_H
