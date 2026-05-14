#ifndef FILE_TREE_PANEL_H
#define FILE_TREE_PANEL_H

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <string>
#include <vector>

wxDECLARE_EVENT(wxEVT_GLANCE_FILE_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GLANCE_FILE_ACTIVATED, wxCommandEvent);

class FileTreePanel : public wxPanel
{
public:
    FileTreePanel(wxWindow* parent);
    virtual ~FileTreePanel();

    bool OpenFolder(const wxString& folderPath);
    void RefreshTree();
    wxString GetCurrentFolder() const;

private:
    void OnTreeItemSelected(wxTreeEvent& event);
    void OnTreeItemActivated(wxTreeEvent& event);
    void SendFileEvent(const wxTreeItemId& item, wxEventType eventType);

    // Helper methods
    bool AddFolderToTree(wxTreeItemId parentItem, const wxString& folderPath);
    void AddFileToTree(wxTreeItemId parentItem, const wxString& displayName, const wxString& filePath);
    bool IsMarkdownFile(const wxString& filePath);
    bool ShouldSkipEntry(const wxString& name) const;

    wxTreeCtrl* m_treeCtrl;
    wxTreeItemId m_rootItem;
    wxString m_currentFolder;

    wxDECLARE_EVENT_TABLE();
};

#endif // FILE_TREE_PANEL_H
