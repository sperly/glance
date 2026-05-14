#include "HelpDialog.h"
#include "EmbeddedResources.h"
#include "MarkdownRenderer.h"
#include <wx/button.h>
#include <wx/html/htmlwin.h>
#include <wx/sizer.h>

namespace
{
wxString BuildHelpHtml(const wxString& body)
{
    return R"(<!doctype html>
<html>
<head>
<meta charset="utf-8">
<style>
body {
    margin: 0;
    padding: 22px 26px;
    color: #1f2933;
    background: #ffffff;
    font: 14px/1.55 -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
}
h1, h2, h3 { color: #132f4c; }
h1 { border-bottom: 1px solid #d8dee6; padding-bottom: 0.25em; }
code {
    padding: 0.12em 0.35em;
    border-radius: 4px;
    background: #edf2f7;
    font-family: "SFMono-Regular", Consolas, monospace;
}
pre {
    overflow: auto;
    padding: 12px;
    border-radius: 6px;
    background: #111827;
}
pre code { color: #f9fafb; background: transparent; }
table { width: 100%; border-collapse: collapse; margin: 1em 0; }
th, td { border: 1px solid #d8dee6; padding: 6px 8px; }
th { background: #f1f5f9; text-align: left; }
a { color: #075985; }
</style>
</head>
<body>
)" + body + R"(
</body>
</html>)";
}
}

HelpDialog::HelpDialog(wxWindow* parent)
    : wxDialog(parent,
               wxID_ANY,
               "Glance Help",
               wxDefaultPosition,
               wxSize(760, 640),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    MarkdownRenderer renderer;
    wxHtmlWindow* helpView = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO);
    helpView->SetPage(BuildHelpHtml(renderer.RenderDocument(wxString::FromUTF8(GetEmbeddedHelpMarkdown()))));

    wxButton* closeButton = new wxButton(this, wxID_CLOSE, "Close");
    closeButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        EndModal(wxID_CLOSE);
    });

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(closeButton, 0);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(helpView, 1, wxEXPAND | wxALL, 8);
    sizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    SetSizerAndFit(sizer);
    SetMinSize(wxSize(620, 420));
}
