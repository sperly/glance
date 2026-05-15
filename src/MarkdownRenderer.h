#ifndef MARKDOWN_RENDERER_H
#define MARKDOWN_RENDERER_H

#include <wx/string.h>

class MarkdownRenderer {
 public:
  wxString RenderDocument(const wxString& markdown,
                          const wxString& sourceFilePath = wxString()) const;

 private:
  wxString RenderInline(const wxString& text,
                        const wxString& baseDirectory = wxString()) const;
};

#endif  // MARKDOWN_RENDERER_H
