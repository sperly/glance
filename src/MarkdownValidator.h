#ifndef MARKDOWN_VALIDATOR_H
#define MARKDOWN_VALIDATOR_H

#include <wx/string.h>

#include <vector>

#include "MarkdownFlavor.h"

enum class MarkdownDiagnosticSeverity {
  Warning,
  Error,
};

struct MarkdownDiagnostic {
  size_t line;
  MarkdownDiagnosticSeverity severity;
  wxString message;
};

class MarkdownValidator {
 public:
  std::vector<MarkdownDiagnostic> Validate(const wxString& markdown,
                                           MarkdownFlavor flavor) const;
  std::vector<MarkdownDiagnostic> Validate(
      const wxString& markdown,
      const MarkdownFlavorDefinition& definition) const;
};

#endif  // MARKDOWN_VALIDATOR_H
