#include "FileTreePanel.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <algorithm>
#include <utility>

wxDEFINE_EVENT(wxEVT_GLANCE_FILE_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_GLANCE_FILE_ACTIVATED, wxCommandEvent);

namespace
{
class FileTreeItemData : public wxTreeItemData
{
public:
    explicit FileTreeItemData(wxString filePath)
        : m_filePath(std::move(filePath))
    {
    }

    const wxString& GetFilePath() const
    {
        return m_filePath;
    }

private:
    wxString m_filePath;
};

struct TreeEntry
{
    wxString name;
    wxString path;
    bool isDirectory;
};
}

wxBEGIN_EVENT_TABLE(FileTreePanel, wxPanel)
    EVT_TREE_SEL_CHANGED(wxID_ANY, FileTreePanel::OnTreeItemSelected)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY, FileTreePanel::OnTreeItemActivated)
wxEND_EVENT_TABLE()

FileTreePanel::FileTreePanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    // Create the tree control
    m_treeCtrl = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT);

    // Create a sizer to manage the layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_treeCtrl, 1, wxEXPAND | wxALL, 5);
    SetSizer(sizer);

    // Initialize the root item
    m_rootItem = m_treeCtrl->AddRoot("Files");
    m_treeCtrl->SetItemData(m_rootItem, nullptr);
}

FileTreePanel::~FileTreePanel()
{
}

bool FileTreePanel::OpenFolder(const wxString& folderPath)
{
    wxFileName folderName = wxFileName::DirName(folderPath);
    folderName.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    const wxString normalizedPath = folderName.GetPath();

    if (!wxDir::Exists(normalizedPath))
    {
        wxMessageBox("Folder does not exist: " + normalizedPath, "Error", wxOK | wxICON_ERROR);
        return false;
    }

    // Clear existing tree
    m_treeCtrl->DeleteAllItems();
    m_rootItem = m_treeCtrl->AddRoot(folderName.GetFullName().empty() ? normalizedPath : folderName.GetFullName());
    m_currentFolder = normalizedPath;

    // Add the folder to the tree
    AddFolderToTree(m_rootItem, normalizedPath);

    return true;
}

void FileTreePanel::RefreshTree()
{
    if (!m_currentFolder.empty())
    {
        OpenFolder(m_currentFolder);
    }
}

wxString FileTreePanel::GetCurrentFolder() const
{
    return m_currentFolder;
}

bool FileTreePanel::AddFolderToTree(wxTreeItemId parentItem, const wxString& folderPath)
{
    wxDir dir(folderPath);
    if (!dir.IsOpened())
        return false;

    wxString filename;
    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS | wxDIR_FILES);

    // Create a vector to store items so we can sort them
    std::vector<TreeEntry> items;

    while (cont)
    {
        if (!ShouldSkipEntry(filename))
        {
            wxFileName fullPath(folderPath, filename);
            bool isDir = wxDir::Exists(fullPath.GetFullPath());
            items.push_back({filename, fullPath.GetFullPath(), isDir});
        }
        cont = dir.GetNext(&filename);
    }

    // Sort items (directories first, then files)
    std::sort(items.begin(), items.end(), [](const TreeEntry& a, const TreeEntry& b) {
        if (a.isDirectory && !b.isDirectory) return true;
        if (!a.isDirectory && b.isDirectory) return false;
        return a.name.CmpNoCase(b.name) < 0;
    });

    bool addedMarkdownFile = false;

    for (const auto& item : items)
    {
        if (item.isDirectory)
        {
            wxTreeItemId dirItem = m_treeCtrl->AppendItem(parentItem, item.name);
            if (AddFolderToTree(dirItem, item.path))
            {
                addedMarkdownFile = true;
            }
            else
            {
                m_treeCtrl->Delete(dirItem);
            }
        }
        else if (IsMarkdownFile(item.name))
        {
            AddFileToTree(parentItem, item.name, item.path);
            addedMarkdownFile = true;
        }
    }

    return addedMarkdownFile;
}

void FileTreePanel::AddFileToTree(wxTreeItemId parentItem, const wxString& displayName, const wxString& filePath)
{
    // This method is called for files that are already known to be Markdown files
    wxTreeItemId fileItem = m_treeCtrl->AppendItem(parentItem, displayName);
    m_treeCtrl->SetItemData(fileItem, new FileTreeItemData(filePath));
}

bool FileTreePanel::IsMarkdownFile(const wxString& filePath)
{
    const wxString extension = wxFileName(filePath).GetExt().Lower();
    return extension == "md" || extension == "markdown" || extension == "mdown" || extension == "mkd";
}

bool FileTreePanel::ShouldSkipEntry(const wxString& name) const
{
    return name.StartsWith(".");
}

void FileTreePanel::OnTreeItemSelected(wxTreeEvent& event)
{
    SendFileEvent(event.GetItem(), wxEVT_GLANCE_FILE_SELECTED);
    event.Skip();
}

void FileTreePanel::OnTreeItemActivated(wxTreeEvent& event)
{
    SendFileEvent(event.GetItem(), wxEVT_GLANCE_FILE_ACTIVATED);
    event.Skip();
}

void FileTreePanel::SendFileEvent(const wxTreeItemId& item, wxEventType eventType)
{
    if (!item.IsOk() || item == m_rootItem)
        return;

    FileTreeItemData* itemData = dynamic_cast<FileTreeItemData*>(m_treeCtrl->GetItemData(item));
    if (!itemData)
        return;

    wxCommandEvent event(eventType, GetId());
    event.SetEventObject(this);
    event.SetString(itemData->GetFilePath());
    wxPostEvent(this, event);
}
