#ifndef DOCUMENT_MANAGER_H
#define DOCUMENT_MANAGER_H

#include <memory>
#include <vector>

#include "Document.h"

class DocumentManager {
 public:
  Document* OpenDocument(const wxString& filePath,
                         wxString* errorMessage = nullptr);
  Document* FindDocument(const wxString& filePath) const;
  bool CloseDocument(Document* document);

  bool SaveDocument(Document* document, wxString* errorMessage = nullptr);
  bool SaveAll(wxString* errorMessage = nullptr);
  bool HasModifiedDocuments() const;

  const std::vector<std::unique_ptr<Document>>& GetDocuments() const;

  static wxString NormalizePath(const wxString& filePath);

 private:
  std::vector<std::unique_ptr<Document>> m_documents;
};

#endif  // DOCUMENT_MANAGER_H
