#ifndef PREVIEW_PANEL_H
#define PREVIEW_PANEL_H

#include "MarkdownRenderer.h"
#ifdef GLANCE_USE_WEBVIEW
#include <wx/webview.h>
#else
#include <wx/html/htmlwin.h>
#endif
#include <wx/panel.h>
#include <wx/timer.h>

class wxHtmlEasyPrinting;

wxDECLARE_EVENT(wxEVT_GLANCE_PREVIEW_ZOOM_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GLANCE_PREVIEW_SOURCE_LINE, wxCommandEvent);

class PreviewPanel : public wxPanel {
 public:
  explicit PreviewPanel(wxWindow* parent);
  ~PreviewPanel() override;

  void ShowMarkdown(const wxString& markdown, const wxString& sourceFilePath,
                    MarkdownFlavor flavor = MarkdownFlavor::GitHub);
  void Clear();
  void ScrollToSourceLine(int sourceLine);
  wxString GetHtmlSource() const;
  bool PrintMarkdown(const wxString& markdown, const wxString& sourceFilePath,
                     const wxString& title,
                     MarkdownFlavor flavor = MarkdownFlavor::GitHub);
  void ZoomIn();
  void ZoomOut();
  void ResetZoom();

 private:
  void ChangeZoom(double delta);
  void OnUpdateTimer(wxTimerEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnMouseWheel(wxMouseEvent& event);
#ifdef GLANCE_USE_WEBVIEW
  void OnScriptMessage(wxWebViewEvent& event);
  void OnTitleChanged(wxWebViewEvent& event);
  void OnPageLoaded(wxWebViewEvent& event);
#endif
  void ApplyZoom();
  void ApplySourceLine();
  void SendZoomChanged();
  void SendSourceLine(int sourceLine);
  wxString BuildPreviewPage() const;
  wxString BuildHtmlPage(const wxString& renderedBody) const;
  wxString GetBasePath() const;
  wxString GetBaseUrl() const;

#ifdef GLANCE_USE_WEBVIEW
  wxWebView* m_webView;
#else
  wxHtmlWindow* m_htmlWindow;
#endif
  wxHtmlEasyPrinting* m_htmlPrinter;
  wxTimer m_updateTimer;
  MarkdownRenderer m_renderer;
  wxString m_pendingMarkdown;
  wxString m_sourceFilePath;
  MarkdownFlavor m_flavor;
  double m_zoomLevel;
  int m_sourceLine;
};

#endif  // PREVIEW_PANEL_H
