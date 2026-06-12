#include "PreviewPanel.h"

#include <wx/filename.h>
#include <wx/html/htmprint.h>
#include <wx/sizer.h>

#include <algorithm>
#include <cmath>

#include "EmbeddedResources.h"

wxDEFINE_EVENT(wxEVT_GLANCE_PREVIEW_ZOOM_CHANGED, wxCommandEvent);

namespace {
constexpr int PreviewDebounceMs = 400;
constexpr double MinZoomLevel = 0.5;
constexpr double MaxZoomLevel = 2.0;
constexpr double ZoomStep = 0.1;
const wxString ZoomTitlePrefix = "GlanceZoom:";

int ScaledPixels(int pixels, double zoomLevel) {
  return std::max(1, static_cast<int>(std::lround(pixels * zoomLevel)));
}

wxString FormatZoom(double zoomLevel) {
  return wxString::Format("%.2f", zoomLevel);
}

wxString EmbeddedUtf8String(const unsigned char* data, std::size_t size) {
  return wxString::FromUTF8(reinterpret_cast<const char*>(data), size);
}
}  // namespace

PreviewPanel::PreviewPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
#ifdef GLANCE_USE_WEBVIEW
      m_webView(wxWebView::New(this, wxID_ANY)),
#else
      m_htmlWindow(new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition,
                                    wxDefaultSize, wxHW_SCROLLBAR_AUTO)),
#endif
      m_htmlPrinter(new wxHtmlEasyPrinting("Glance Markdown Editor", this)),
      m_updateTimer(this),
      m_flavor(MarkdownFlavor::GitHub),
      m_zoomLevel(1.0) {
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
#ifdef GLANCE_USE_WEBVIEW
  sizer->Add(m_webView, 1, wxEXPAND);
  if (m_webView->CanSetZoomType(wxWEBVIEW_ZOOM_TYPE_LAYOUT)) {
    m_webView->SetZoomType(wxWEBVIEW_ZOOM_TYPE_LAYOUT);
  }
  m_webView->AddScriptMessageHandler("glance");
  m_webView->Bind(wxEVT_KEY_DOWN, &PreviewPanel::OnKeyDown, this);
  m_webView->Bind(wxEVT_MOUSEWHEEL, &PreviewPanel::OnMouseWheel, this);
  m_webView->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED,
                  &PreviewPanel::OnScriptMessage, this);
  m_webView->Bind(wxEVT_WEBVIEW_TITLE_CHANGED, &PreviewPanel::OnTitleChanged,
                  this);
#else
  sizer->Add(m_htmlWindow, 1, wxEXPAND);
  m_htmlWindow->Bind(wxEVT_KEY_DOWN, &PreviewPanel::OnKeyDown, this);
  m_htmlWindow->Bind(wxEVT_MOUSEWHEEL, &PreviewPanel::OnMouseWheel, this);
#endif
  SetSizer(sizer);

  Bind(wxEVT_TIMER, &PreviewPanel::OnUpdateTimer, this, m_updateTimer.GetId());
  Clear();
}

PreviewPanel::~PreviewPanel() {
#ifdef GLANCE_USE_WEBVIEW
  m_webView->RemoveScriptMessageHandler("glance");
#endif
  delete m_htmlPrinter;
}

void PreviewPanel::ShowMarkdown(const wxString& markdown,
                                const wxString& sourceFilePath,
                                MarkdownFlavor flavor) {
  m_pendingMarkdown = markdown;
  m_sourceFilePath = sourceFilePath;
  m_flavor = flavor;
  m_updateTimer.StartOnce(PreviewDebounceMs);
}

void PreviewPanel::Clear() {
  m_pendingMarkdown.clear();
  m_sourceFilePath.clear();
  m_flavor = MarkdownFlavor::GitHub;
  m_updateTimer.Stop();
#ifdef GLANCE_USE_WEBVIEW
  m_webView->SetPage(BuildHtmlPage("<p class=\"empty\">No document open</p>"),
                     "");
  ApplyZoom();
#else
  m_htmlWindow->SetPage(
      BuildHtmlPage("<p class=\"empty\">No document open</p>"));
#endif
}

wxString PreviewPanel::GetHtmlSource() const {
  if (m_pendingMarkdown.empty() && m_sourceFilePath.empty()) {
    return BuildHtmlPage("<p class=\"empty\">No document open</p>");
  }

  return BuildHtmlPage(
      m_renderer.RenderDocument(m_pendingMarkdown, m_sourceFilePath, m_flavor));
}

bool PreviewPanel::PrintMarkdown(const wxString& markdown,
                                 const wxString& sourceFilePath,
                                 const wxString& title, MarkdownFlavor flavor) {
  m_pendingMarkdown = markdown;
  m_sourceFilePath = sourceFilePath;
  m_flavor = flavor;
  m_updateTimer.Stop();

  const wxString html = GetHtmlSource();
#ifdef GLANCE_USE_WEBVIEW
  m_webView->SetPage(html, GetBaseUrl());
  ApplyZoom();
#else
  m_htmlWindow->SetPage(html);
#endif
  m_htmlPrinter->SetName(title.empty() ? "Glance Markdown Editor" : title);
  return m_htmlPrinter->PrintText(html, GetBasePath());
}

void PreviewPanel::OnUpdateTimer(wxTimerEvent& event) {
#ifdef GLANCE_USE_WEBVIEW
  m_webView->SetPage(GetHtmlSource(), GetBaseUrl());
  ApplyZoom();
#else
  m_htmlWindow->SetPage(GetHtmlSource());
#endif
}

void PreviewPanel::ZoomIn() { ChangeZoom(ZoomStep); }

void PreviewPanel::ZoomOut() { ChangeZoom(-ZoomStep); }

void PreviewPanel::ResetZoom() {
  m_zoomLevel = 1.0;
  ApplyZoom();
  SendZoomChanged();
}

void PreviewPanel::ChangeZoom(double delta) {
  m_zoomLevel = std::clamp(m_zoomLevel + delta, MinZoomLevel, MaxZoomLevel);
  ApplyZoom();
  SendZoomChanged();
}

void PreviewPanel::OnKeyDown(wxKeyEvent& event) {
  if (event.ControlDown()) {
    if (event.GetKeyCode() == '+' || event.GetKeyCode() == '=') {
      ZoomIn();
      return;
    } else if (event.GetKeyCode() == '-') {
      ZoomOut();
      return;
    } else if (event.GetKeyCode() == '0') {
      ResetZoom();
      return;
    }
  }
  event.Skip();
}

void PreviewPanel::OnMouseWheel(wxMouseEvent& event) {
  if (event.ControlDown()) {
    if (event.GetWheelRotation() > 0) {
      ZoomIn();
    } else if (event.GetWheelRotation() < 0) {
      ZoomOut();
    }
  } else {
    event.Skip();
  }
}

#ifdef GLANCE_USE_WEBVIEW
void PreviewPanel::OnScriptMessage(wxWebViewEvent& event) {
  if (event.GetMessageHandler() != "glance") {
    event.Skip();
    return;
  }

  const wxString message = event.GetString();
  if (message.StartsWith("zoom-set:")) {
    double zoomLevel = m_zoomLevel;
    if (message.Mid(9).ToDouble(&zoomLevel)) {
      m_zoomLevel = std::clamp(zoomLevel, MinZoomLevel, MaxZoomLevel);
      SendZoomChanged();
    }
  }
}

void PreviewPanel::OnTitleChanged(wxWebViewEvent& event) {
  const wxString title = event.GetString();
  if (!title.StartsWith(ZoomTitlePrefix)) {
    event.Skip();
    return;
  }

  long zoomPercent = 100;
  if (!title.Mid(ZoomTitlePrefix.length()).ToLong(&zoomPercent)) {
    return;
  }

  m_zoomLevel = std::clamp(static_cast<double>(zoomPercent) / 100.0,
                           MinZoomLevel, MaxZoomLevel);
  SendZoomChanged();
}
#endif

void PreviewPanel::ApplyZoom() {
#ifdef GLANCE_USE_WEBVIEW
  m_webView->RunScript(wxString::Format(
      "if (window.glanceApplyPreviewZoom) { "
      "window.glanceApplyPreviewZoom(%s); "
      "} else { "
      "document.documentElement.style.setProperty('--glance-preview-zoom', "
      "'%s'); "
      "if (document.body) document.body.style.zoom = '%s'; "
      "}",
      FormatZoom(m_zoomLevel), FormatZoom(m_zoomLevel),
      FormatZoom(m_zoomLevel)));
#else
  m_htmlWindow->SetPage(GetHtmlSource());
#endif
}

void PreviewPanel::SendZoomChanged() {
  wxCommandEvent event(wxEVT_GLANCE_PREVIEW_ZOOM_CHANGED, GetId());
  event.SetEventObject(this);
  event.SetInt(static_cast<int>(std::lround(m_zoomLevel * 100.0)));
  wxPostEvent(this, event);
}

wxString PreviewPanel::BuildHtmlPage(const wxString& renderedBody) const {
#ifdef GLANCE_USE_WEBVIEW
  const wxString bodyPaddingCss = "24px 28px";
  const wxString bodyFontCss =
      "15px/1.55 -apple-system, "
      "BlinkMacSystemFont, \"Segoe UI\", sans-serif";
  const wxString bodyZoomCss = "    zoom: var(--glance-preview-zoom);\n";
#else
  const wxString bodyPaddingCss =
      wxString::Format("%dpx %dpx", ScaledPixels(24, m_zoomLevel),
                       ScaledPixels(28, m_zoomLevel));
  const wxString bodyFontCss = wxString::Format(
      "%dpx/1.55 -apple-system, BlinkMacSystemFont, \"Segoe UI\", sans-serif",
      ScaledPixels(15, m_zoomLevel));
  const wxString bodyZoomCss;
#endif

  wxString page =
      R"(<!doctype html>
<html>
<head>
<meta charset="utf-8">
<style>
:root {
    --glance-preview-zoom: )" +
      FormatZoom(m_zoomLevel) +
      R"(;
}
body {
    box-sizing: border-box;
    margin: 0;
    padding: )" +
      bodyPaddingCss +
      R"(;
    color: #1f2933;
    background: #ffffff;
    font: )" +
      bodyFontCss +
      R"(;
)" + bodyZoomCss +
      R"(
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
.plantuml-diagram { overflow: auto; margin: 1em 0; text-align: center; }
.plantuml-diagram svg { max-width: 100%; height: auto; }
hr { border: 0; border-top: 1px solid #d8dee6; margin: 1.5em 0; }
.glance-strike { text-decoration: line-through; }
mark { background: #fff3a3; color: inherit; padding: 0.05em 0.2em; border-radius: 3px; }
.task { list-style: none; margin-left: -1.2em; }
.empty { color: #6b7280; }
)" +
      EmbeddedUtf8String(GetEmbeddedHighlightCssData(),
                         GetEmbeddedHighlightCssSize()) +
      R"(
</style>
</head>
<body>
)" + renderedBody +
      R"(
)";
#ifdef GLANCE_USE_WEBVIEW
  page += R"(
<script>
window.glancePreviewZoom = )" +
          FormatZoom(m_zoomLevel) +
          R"(;

function postGlanceMessage(message) {
  if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.glance) {
    window.webkit.messageHandlers.glance.postMessage(message);
  }
}

window.glanceApplyPreviewZoom = function(zoom) {
  zoom = Math.max(0.5, Math.min(2.0, zoom));
  window.glancePreviewZoom = zoom;
  document.documentElement.style.setProperty('--glance-preview-zoom', zoom.toFixed(2));
  document.body.style.zoom = zoom.toFixed(2);
  document.title = 'GlanceZoom:' + Math.round(zoom * 100);
};

function changePreviewZoom(delta) {
  var zoom = Math.round((window.glancePreviewZoom + delta) * 10) / 10;
  window.glanceApplyPreviewZoom(zoom);
  postGlanceMessage('zoom-set:' + window.glancePreviewZoom.toFixed(2));
}

document.addEventListener('wheel', function(event) {
  if (event.ctrlKey) {
    event.preventDefault();
    changePreviewZoom(event.deltaY < 0 ? 0.1 : -0.1);
  }
}, { passive: false });

document.addEventListener('keydown', function(event) {
  if (!event.ctrlKey) {
    return;
  }

  if (event.key === '+' || event.key === '=') {
    event.preventDefault();
    changePreviewZoom(0.1);
  } else if (event.key === '-') {
    event.preventDefault();
    changePreviewZoom(-0.1);
  } else if (event.key === '0') {
    event.preventDefault();
    window.glanceApplyPreviewZoom(1.0);
    postGlanceMessage('zoom-set:1.00');
  }
});
</script>
<script>
)" +
          EmbeddedUtf8String(GetEmbeddedHighlightJsData(),
                             GetEmbeddedHighlightJsSize()) +
          R"(
</script>
<script>
hljs.highlightAll();
</script>
)";
#else
  page += R"(
<script>
// Prevent scrolling when Ctrl+Wheel is pressed to allow zoom handling
document.addEventListener('wheel', function(event) {
  if (event.ctrlKey) {
    event.preventDefault();
  }
}, { passive: false });
</script>
)";
#endif
  page += R"(
</body>
</html>)";

  return page;
}

wxString PreviewPanel::GetBasePath() const {
  if (m_sourceFilePath.empty()) {
    return wxString();
  }

  return wxFileName(m_sourceFilePath).GetPath();
}

wxString PreviewPanel::GetBaseUrl() const {
  if (m_sourceFilePath.empty()) {
    return wxString();
  }

  wxFileName fileName(m_sourceFilePath);
  return "file://" + fileName.GetPathWithSep();
}
