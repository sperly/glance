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

class PreviewPanel : public wxPanel {
 public:
  explicit PreviewPanel(wxWindow* parent);
  ~PreviewPanel() override;

  void ShowMarkdown(const wxString& markdown, const wxString& sourceFilePath,
                    MarkdownFlavor flavor = MarkdownFlavor::GitHub);
  void Clear();
  wxString GetHtmlSource() const;
  bool PrintMarkdown(const wxString& markdown, const wxString& sourceFilePath,
                     const wxString& title,
                     MarkdownFlavor flavor = MarkdownFlavor::GitHub);

 private:
  void OnUpdateTimer(wxTimerEvent& event);
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
};

#endif  // PREVIEW_PANEL_H
