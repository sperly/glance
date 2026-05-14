#include "Document.h"
#include <wx/file.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <utility>

Document::Document(wxString filePath)
    : m_filePath(std::move(filePath)),
      m_modified(false)
{
}

const wxString& Document::GetFilePath() const
{
    return m_filePath;
}

wxString Document::GetFileName() const
{
    return wxFileName(m_filePath).GetFullName();
}

const wxString& Document::GetContent() const
{
    return m_content;
}

bool Document::IsModified() const
{
    return m_modified;
}

void Document::SetContent(const wxString& content)
{
    m_content = content;
}

void Document::SetModified(bool modified)
{
    m_modified = modified;
}

bool Document::Load(wxString* errorMessage)
{
    if (m_filePath.empty())
    {
        if (errorMessage)
        {
            *errorMessage = "No file path was provided.";
        }
        return false;
    }

    if (!wxFileName::FileExists(m_filePath))
    {
        if (errorMessage)
        {
            *errorMessage = "File does not exist: " + m_filePath;
        }
        return false;
    }

    wxFile file(m_filePath);
    if (!file.IsOpened())
    {
        if (errorMessage)
        {
            *errorMessage = "Unable to open file: " + m_filePath;
        }
        return false;
    }

    wxString content;
    if (!file.ReadAll(&content, wxConvUTF8))
    {
        if (errorMessage)
        {
            *errorMessage = "Unable to read file as UTF-8: " + m_filePath;
        }
        return false;
    }

    m_content = content;
    m_modified = false;
    MarkDiskStateCurrent();
    return true;
}

bool Document::Save(wxString* errorMessage)
{
    if (m_filePath.empty())
    {
        if (errorMessage)
        {
            *errorMessage = "No file path was provided.";
        }
        return false;
    }

    wxFileName fileName(m_filePath);
    if (!wxFileName::DirExists(fileName.GetPath()))
    {
        if (errorMessage)
        {
            *errorMessage = "Folder does not exist: " + fileName.GetPath();
        }
        return false;
    }

    wxFile file(m_filePath, wxFile::write);
    if (!file.IsOpened())
    {
        if (errorMessage)
        {
            *errorMessage = "Unable to write file: " + m_filePath;
        }
        return false;
    }

    if (!file.Write(m_content, wxConvUTF8))
    {
        if (errorMessage)
        {
            *errorMessage = "Unable to save file: " + m_filePath;
        }
        return false;
    }

    m_modified = false;
    MarkDiskStateCurrent();
    return true;
}

bool Document::HasChangedOnDisk() const
{
    const wxDateTime currentModificationTime = GetDiskModificationTime();
    if (!currentModificationTime.IsValid() || !m_lastKnownModificationTime.IsValid())
    {
        return false;
    }

    return !currentModificationTime.IsEqualTo(m_lastKnownModificationTime);
}

void Document::MarkDiskStateCurrent()
{
    m_lastKnownModificationTime = GetDiskModificationTime();
}

wxDateTime Document::GetDiskModificationTime() const
{
    if (m_filePath.empty() || !wxFileName::FileExists(m_filePath))
    {
        return wxDateTime();
    }

    return wxFileName(m_filePath).GetModificationTime();
}
