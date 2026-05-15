#ifndef DOCUMENT_SETTINGS_DIALOG_H
#define DOCUMENT_SETTINGS_DIALOG_H

#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/stattext.h>

#include "MarkdownFlavor.h"

class DocumentSettingsDialog : public wxDialog {
 public:
  DocumentSettingsDialog(wxWindow* parent, MarkdownFlavor flavor);

  MarkdownFlavor GetSelectedFlavor() const;

 private:
  void OnFlavorChanged(wxCommandEvent& event);
  void UpdateDescription();

  wxChoice* m_flavorChoice;
  wxStaticText* m_descriptionLabel;
};

#endif  // DOCUMENT_SETTINGS_DIALOG_H
