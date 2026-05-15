#include "DocumentSettingsDialog.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>

DocumentSettingsDialog::DocumentSettingsDialog(wxWindow* parent,
                                               MarkdownFlavor flavor)
    : wxDialog(parent, wxID_ANY, "Document Settings", wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_flavorChoice(nullptr),
      m_descriptionLabel(nullptr) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

  mainSizer->Add(new wxStaticText(this, wxID_ANY, "Markdown flavor:"), 0,
                 wxLEFT | wxRIGHT | wxTOP, 12);

  m_flavorChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition,
                                wxDefaultSize, GetMarkdownFlavorDisplayNames());
  m_flavorChoice->SetSelection(GetMarkdownFlavorDisplayIndex(flavor));
  mainSizer->Add(m_flavorChoice, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 12);

  m_descriptionLabel = new wxStaticText(this, wxID_ANY, wxString());
  m_descriptionLabel->Wrap(360);
  mainSizer->Add(m_descriptionLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP,
                 12);

  mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 12);

  wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
  buttonSizer->AddButton(new wxButton(this, wxID_OK));
  buttonSizer->AddButton(new wxButton(this, wxID_CANCEL));
  buttonSizer->Realize();
  mainSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

  SetSizerAndFit(mainSizer);
  SetMinSize(GetSize());

  Bind(wxEVT_CHOICE, &DocumentSettingsDialog::OnFlavorChanged, this,
       m_flavorChoice->GetId());
  UpdateDescription();
}

MarkdownFlavor DocumentSettingsDialog::GetSelectedFlavor() const {
  return MarkdownFlavorFromDisplayIndex(m_flavorChoice->GetSelection());
}

void DocumentSettingsDialog::OnFlavorChanged(wxCommandEvent& event) {
  UpdateDescription();
}

void DocumentSettingsDialog::UpdateDescription() {
  const MarkdownFlavorDefinition& definition =
      GetMarkdownFlavorDefinition(GetSelectedFlavor());
  m_descriptionLabel->SetLabel(definition.description);
  Layout();
}
