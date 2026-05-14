#include "DocumentManager.h"
#include <wx/filename.h>
#include <algorithm>

Document* DocumentManager::OpenDocument(const wxString& filePath, wxString* errorMessage)
{
    const wxString normalizedPath = NormalizePath(filePath);
    if (Document* existingDocument = FindDocument(normalizedPath))
    {
        return existingDocument;
    }

    auto document = std::make_unique<Document>(normalizedPath);
    if (!document->Load(errorMessage))
    {
        return nullptr;
    }

    Document* rawDocument = document.get();
    m_documents.push_back(std::move(document));
    return rawDocument;
}

Document* DocumentManager::FindDocument(const wxString& filePath) const
{
    const wxString normalizedPath = NormalizePath(filePath);
    for (const auto& document : m_documents)
    {
        if (document->GetFilePath() == normalizedPath)
        {
            return document.get();
        }
    }

    return nullptr;
}

bool DocumentManager::CloseDocument(Document* document)
{
    auto it = std::find_if(m_documents.begin(), m_documents.end(),
                           [document](const std::unique_ptr<Document>& candidate) {
                               return candidate.get() == document;
                           });
    if (it == m_documents.end())
    {
        return false;
    }

    m_documents.erase(it);
    return true;
}

bool DocumentManager::SaveDocument(Document* document, wxString* errorMessage)
{
    return document && document->Save(errorMessage);
}

bool DocumentManager::SaveAll(wxString* errorMessage)
{
    for (const auto& document : m_documents)
    {
        if (document->IsModified() && !document->Save(errorMessage))
        {
            return false;
        }
    }

    return true;
}

bool DocumentManager::HasModifiedDocuments() const
{
    return std::any_of(m_documents.begin(), m_documents.end(),
                       [](const std::unique_ptr<Document>& document) {
                           return document->IsModified();
                       });
}

const std::vector<std::unique_ptr<Document>>& DocumentManager::GetDocuments() const
{
    return m_documents;
}

wxString DocumentManager::NormalizePath(const wxString& filePath)
{
    wxFileName normalized(filePath);
    normalized.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    return normalized.GetFullPath();
}
