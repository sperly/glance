#include "SearchReplaceDialog.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "EditorNotebook.h"

namespace {
enum { ID_FIND_NEXT = wxID_HIGHEST + 200, ID_REPLACE, ID_REPLACE_ALL };
}  // namespace

SearchReplaceDialog::SearchReplaceDialog(wxWindow* parent,
                                         EditorNotebook* editorNotebook)
    : wxDialog(parent, wxID_ANY, "Search and Replace", wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_editorNotebook(editorNotebook),
      m_findText(new wxTextCtrl(this, wxID_ANY)),
      m_replaceText(new wxTextCtrl(this, wxID_ANY)),
      m_matchCase(new wxCheckBox(this, wxID_ANY, "Match case")),
      m_wrapSearch(new wxCheckBox(this, wxID_ANY, "Wrap search")),
      m_statusLabel(new wxStaticText(this, wxID_ANY, wxString())) {
  m_wrapSearch->SetValue(true);

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer* fieldSizer = new wxFlexGridSizer(2, 8, 8);
  fieldSizer->AddGrowableCol(1, 1);
  fieldSizer->Add(new wxStaticText(this, wxID_ANY, "Find:"), 0,
                  wxALIGN_CENTER_VERTICAL);
  fieldSizer->Add(m_findText, 1, wxEXPAND);
  fieldSizer->Add(new wxStaticText(this, wxID_ANY, "Replace:"), 0,
                  wxALIGN_CENTER_VERTICAL);
  fieldSizer->Add(m_replaceText, 1, wxEXPAND);

  wxBoxSizer* optionSizer = new wxBoxSizer(wxHORIZONTAL);
  optionSizer->Add(m_matchCase, 0, wxRIGHT, 16);
  optionSizer->Add(m_wrapSearch, 0);

  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, ID_FIND_NEXT, "Find Next"), 0, wxRIGHT,
                   8);
  buttonSizer->Add(new wxButton(this, ID_REPLACE, "Replace"), 0, wxRIGHT, 8);
  buttonSizer->Add(new wxButton(this, ID_REPLACE_ALL, "Replace All"), 0,
                   wxRIGHT, 8);
  buttonSizer->AddStretchSpacer();
  buttonSizer->Add(new wxButton(this, wxID_CLOSE, "Close"), 0);

  mainSizer->Add(fieldSizer, 0, wxEXPAND | wxALL, 12);
  mainSizer->Add(optionSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
  mainSizer->Add(m_statusLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
  mainSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

  SetSizerAndFit(mainSizer);
  SetMinSize(wxSize(420, GetSize().GetHeight()));

  Bind(wxEVT_BUTTON, &SearchReplaceDialog::OnFindNext, this, ID_FIND_NEXT);
  Bind(wxEVT_BUTTON, &SearchReplaceDialog::OnReplace, this, ID_REPLACE);
  Bind(wxEVT_BUTTON, &SearchReplaceDialog::OnReplaceAll, this, ID_REPLACE_ALL);
  Bind(wxEVT_BUTTON, &SearchReplaceDialog::OnClose, this, wxID_CLOSE);

  m_findText->SetFocus();
}

void SearchReplaceDialog::OnFindNext(wxCommandEvent& event) {
  if (GetSearchText().empty()) {
    SetStatus("Enter text to find.");
    return;
  }

  if (!m_editorNotebook || !m_editorNotebook->FindNextText(
                               GetSearchText(), MatchCase(), WrapSearch())) {
    SetStatus("No match found.");
    return;
  }

  SetStatus("Match found.");
}

void SearchReplaceDialog::OnReplace(wxCommandEvent& event) {
  if (GetSearchText().empty()) {
    SetStatus("Enter text to find.");
    return;
  }

  if (!m_editorNotebook ||
      !m_editorNotebook->ReplaceNextText(GetSearchText(), GetReplacementText(),
                                         MatchCase(), WrapSearch())) {
    SetStatus("No match found.");
    return;
  }

  SetStatus("Replaced current match.");
}

void SearchReplaceDialog::OnReplaceAll(wxCommandEvent& event) {
  if (GetSearchText().empty()) {
    SetStatus("Enter text to find.");
    return;
  }

  const int replacements =
      m_editorNotebook ? m_editorNotebook->ReplaceAllText(
                             GetSearchText(), GetReplacementText(), MatchCase())
                       : 0;
  SetStatus(wxString::Format("Replaced %d match%s.", replacements,
                             replacements == 1 ? "" : "es"));
}

void SearchReplaceDialog::OnClose(wxCommandEvent& event) { Destroy(); }

void SearchReplaceDialog::SetStatus(const wxString& message) {
  m_statusLabel->SetLabel(message);
}

wxString SearchReplaceDialog::GetSearchText() const {
  return m_findText->GetValue();
}

wxString SearchReplaceDialog::GetReplacementText() const {
  return m_replaceText->GetValue();
}

bool SearchReplaceDialog::MatchCase() const { return m_matchCase->GetValue(); }

bool SearchReplaceDialog::WrapSearch() const {
  return m_wrapSearch->GetValue();
}
