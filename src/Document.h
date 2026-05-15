#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <wx/datetime.h>
#include <wx/string.h>

#include "MarkdownFlavor.h"

class Document {
 public:
  explicit Document(wxString filePath = wxString());

  const wxString& GetFilePath() const;
  wxString GetFileName() const;
  const wxString& GetContent() const;
  MarkdownFlavor GetMarkdownFlavor() const;
  bool IsModified() const;

  void SetContent(const wxString& content);
  void SetMarkdownFlavor(MarkdownFlavor flavor);
  void SetModified(bool modified);

  bool Load(wxString* errorMessage = nullptr);
  bool Save(wxString* errorMessage = nullptr);
  bool HasChangedOnDisk() const;
  void MarkDiskStateCurrent();

 private:
  wxDateTime GetDiskModificationTime() const;

  wxString m_filePath;
  wxString m_content;
  wxDateTime m_lastKnownModificationTime;
  MarkdownFlavor m_markdownFlavor;
  bool m_modified;
};

#endif  // DOCUMENT_H
