#ifndef MARKDOWN_RENDERER_H
#define MARKDOWN_RENDERER_H

#include <wx/string.h>

#include "MarkdownFlavor.h"

class MarkdownRenderer {
 public:
  MarkdownRenderer();
  explicit MarkdownRenderer(const wxString& plantUmlExecutable);

  wxString RenderDocument(const wxString& markdown,
                          const wxString& sourceFilePath = wxString(),
                          MarkdownFlavor flavor = MarkdownFlavor::GitHub) const;
  wxString RenderDocument(const wxString& markdown,
                          const wxString& sourceFilePath,
                          const MarkdownFlavorDefinition& definition) const;

 private:
  wxString RenderPlantUml(const wxString& source) const;
  wxString RenderInline(const wxString& text, const wxString& baseDirectory,
                        const MarkdownFlavorDefinition& definition) const;

  wxString m_plantUmlExecutable;
};

#endif  // MARKDOWN_RENDERER_H
