#include "PreviewPanel.h"
#include <wx/filename.h>
#include <wx/sizer.h>

namespace
{
constexpr int PreviewDebounceMs = 400;
}

PreviewPanel::PreviewPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
#ifdef GLANCE_USE_WEBVIEW
      m_webView(wxWebView::New(this, wxID_ANY)),
#else
      m_htmlWindow(new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO)),
#endif
      m_updateTimer(this)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
#ifdef GLANCE_USE_WEBVIEW
    sizer->Add(m_webView, 1, wxEXPAND);
#else
    sizer->Add(m_htmlWindow, 1, wxEXPAND);
#endif
    SetSizer(sizer);

    Bind(wxEVT_TIMER, &PreviewPanel::OnUpdateTimer, this, m_updateTimer.GetId());
    Clear();
}

void PreviewPanel::ShowMarkdown(const wxString& markdown, const wxString& sourceFilePath)
{
    m_pendingMarkdown = markdown;
    m_sourceFilePath = sourceFilePath;
    m_updateTimer.StartOnce(PreviewDebounceMs);
}

void PreviewPanel::Clear()
{
    m_pendingMarkdown.clear();
    m_sourceFilePath.clear();
    m_updateTimer.Stop();
#ifdef GLANCE_USE_WEBVIEW
    m_webView->SetPage(BuildHtmlPage("<p class=\"empty\">No document open</p>"), "");
#else
    m_htmlWindow->SetPage(BuildHtmlPage("<p class=\"empty\">No document open</p>"));
#endif
}

wxString PreviewPanel::GetHtmlSource() const
{
    if (m_pendingMarkdown.empty() && m_sourceFilePath.empty())
    {
        return BuildHtmlPage("<p class=\"empty\">No document open</p>");
    }

    return BuildHtmlPage(m_renderer.RenderDocument(m_pendingMarkdown, m_sourceFilePath));
}

void PreviewPanel::OnUpdateTimer(wxTimerEvent& event)
{
#ifdef GLANCE_USE_WEBVIEW
    m_webView->SetPage(GetHtmlSource(), GetBaseUrl());
#else
    m_htmlWindow->SetPage(GetHtmlSource());
#endif
}

wxString PreviewPanel::BuildHtmlPage(const wxString& renderedBody) const
{
    return R"(<!doctype html>
<html>
<head>
<meta charset="utf-8">
<style>
body {
    box-sizing: border-box;
    margin: 0;
    padding: 24px 28px;
    color: #1f2933;
    background: #ffffff;
    font: 15px/1.55 -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
}
body > *:first-child { margin-top: 0; }
body > *:last-child { margin-bottom: 0; }
h1, h2, h3, h4, h5, h6 {
    color: #132f4c;
    line-height: 1.25;
    margin: 1.35em 0 0.45em;
}
h1 { border-bottom: 1px solid #d8dee6; padding-bottom: 0.25em; }
a { color: #075985; }
blockquote {
    margin: 1em 0;
    padding: 0.1em 1em;
    color: #52616f;
    border-left: 4px solid #9fb3c8;
    background: #f6f8fa;
}
code {
    padding: 0.15em 0.35em;
    border-radius: 4px;
    background: #edf2f7;
    font-family: "SFMono-Regular", Consolas, monospace;
}
pre {
    overflow: auto;
    padding: 14px;
    border-radius: 6px;
    background: #111827;
}
pre code {
    padding: 0;
    color: #f9fafb;
    background: transparent;
}
table {
    width: 100%;
    border-collapse: collapse;
    margin: 1em 0;
}
th, td {
    border: 1px solid #d8dee6;
    padding: 6px 8px;
}
th { background: #f1f5f9; text-align: left; }
img { max-width: 100%; height: auto; }
hr { border: 0; border-top: 1px solid #d8dee6; margin: 1.5em 0; }
.glance-strike { text-decoration: line-through; }
.task { list-style: none; margin-left: -1.2em; }
.empty { color: #6b7280; }
</style>
</head>
<body>
)" + renderedBody + R"(
</body>
</html>)";
}

wxString PreviewPanel::GetBaseUrl() const
{
    if (m_sourceFilePath.empty())
    {
        return wxString();
    }

    wxFileName fileName(m_sourceFilePath);
    return "file://" + fileName.GetPathWithSep();
}
