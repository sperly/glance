#include "AboutDialog.h"
#include "EmbeddedResources.h"
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/version.h>

AboutDialog::AboutDialog(wxWindow* parent)
    : wxDialog(parent,
               wxID_ANY,
               "About Glance",
               wxDefaultPosition,
               wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxMemoryInputStream logoStream(GetEmbeddedGlanceLogoPngData(), GetEmbeddedGlanceLogoPngSize());
    wxImage logoImage(logoStream, wxBITMAP_TYPE_PNG);
    if (logoImage.IsOk())
    {
        logoImage.Rescale(128, 128, wxIMAGE_QUALITY_HIGH);
        wxStaticBitmap* logo = new wxStaticBitmap(this, wxID_ANY, wxBitmap(logoImage));
        sizer->Add(logo, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxLEFT | wxRIGHT, 16);
    }

    wxStaticText* title = new wxStaticText(this, wxID_ANY, "Glance Markdown Editor");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 4);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);

    const wxString details =
        "Version: 0.1.0\n"
        "A portable desktop application for editing Markdown files.\n\n"
        "Built with wxWidgets " + wxString(wxVERSION_STRING) + "\n"
        "Language: C++17\n"
        "License: Project is licensed under the MIT license.";

    wxStaticText* body = new wxStaticText(this, wxID_ANY, details);
    wxButton* closeButton = new wxButton(this, wxID_OK, "OK");

    sizer->Add(title, 0, wxALL, 16);
    sizer->Add(body, 0, wxLEFT | wxRIGHT | wxBOTTOM, 16);
    sizer->Add(closeButton, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM, 16);

    SetSizerAndFit(sizer);
    CentreOnParent();
}
