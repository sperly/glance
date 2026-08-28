#ifndef MARKDOWN_RENDERER_H
#define MARKDOWN_RENDERER_H

#include <wx/string.h>

#include "MarkdownFlavor.h"

class MarkdownRenderer {
 public:
  MarkdownRenderer() = default;
  explicit MarkdownRenderer(const wxString& plantUmlExecutable);
  MarkdownRenderer(const wxString& plantUmlExecutable,
                   const wxString& mermaidExecutable);

  static wxString FindPlantUmlExecutable();
  static wxString FindMermaidExecutable();

  wxString RenderDocument(const wxString& markdown,
                          const wxString& sourceFilePath = wxString(),
                          MarkdownFlavor flavor = MarkdownFlavor::GitHub,
                          bool trackSourceLines = false) const;
  wxString RenderDocument(const wxString& markdown,
                          const wxString& sourceFilePath,
                          const MarkdownFlavorDefinition& definition,
                          bool trackSourceLines = false) const;

 private:
  wxString ResolvePlantUmlExecutable() const;
  wxString ResolveMermaidExecutable() const;
  wxString RenderPlantUml(const wxString& source) const;
  wxString RenderMermaid(const wxString& source) const;
  wxString RenderInline(const wxString& text, const wxString& baseDirectory,
                        const MarkdownFlavorDefinition& definition) const;

  wxString m_plantUmlExecutable;
  wxString m_mermaidExecutable;
};

#endif  // MARKDOWN_RENDERER_H
