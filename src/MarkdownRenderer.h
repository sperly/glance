#ifndef MARKDOWN_RENDERER_H
#define MARKDOWN_RENDERER_H

#include <wx/string.h>

#include "MarkdownFlavor.h"

class MarkdownRenderer {
 public:
  wxString RenderDocument(const wxString& markdown,
                          const wxString& sourceFilePath = wxString(),
                          MarkdownFlavor flavor = MarkdownFlavor::GitHub) const;
  wxString RenderDocument(const wxString& markdown,
                          const wxString& sourceFilePath,
                          const MarkdownFlavorDefinition& definition) const;

 private:
  wxString RenderInline(const wxString& text, const wxString& baseDirectory,
                        const MarkdownFlavorDefinition& definition) const;
};

#endif  // MARKDOWN_RENDERER_H
