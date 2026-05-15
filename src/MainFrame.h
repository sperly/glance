#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include <wx/arrstr.h>
#include <wx/splitter.h>
#include <wx/wx.h>

#include "GlanceCtrl.h"
#include "SettingsManager.h"

class FileTreePanel;
class EditorNotebook;
class PreviewPanel;
class wxMenu;

class MainFrame : public wxFrame {
 public:
  MainFrame();
  virtual ~MainFrame();

  bool OpenFolder(const wxString& folderPath);
  bool OpenFile(const wxString& filePath);

 private:
  void CreateMenuBar();

  // Menu event handlers
  void OnFileNewFile(wxCommandEvent& event);
  void OnFileOpenFolder(wxCommandEvent& event);
  void OnFileOpenFile(wxCommandEvent& event);
  void OnFileSave(wxCommandEvent& event);
  void OnFileSaveAll(wxCommandEvent& event);
  void OnFileCloseTab(wxCommandEvent& event);
  void OnFilePrint(wxCommandEvent& event);
  void OnFileExit(wxCommandEvent& event);
  void OnRecentFile(wxCommandEvent& event);
  void OnRecentFolder(wxCommandEvent& event);
  void OnClearRecentItems(wxCommandEvent& event);
  void OnEditUndo(wxCommandEvent& event);
  void OnEditRedo(wxCommandEvent& event);
  void OnEditCut(wxCommandEvent& event);
  void OnEditCopy(wxCommandEvent& event);
  void OnEditPaste(wxCommandEvent& event);
  void OnEditSelectAll(wxCommandEvent& event);
  void OnFormatCommand(wxCommandEvent& event);
  void OnInsertCommand(wxCommandEvent& event);
  void OnDocumentSettings(wxCommandEvent& event);
  void OnDocumentValidate(wxCommandEvent& event);
  void OnHelpShowHelp(wxCommandEvent& event);
  void OnHelpSavePreviewHtml(wxCommandEvent& event);
  void OnHelpAbout(wxCommandEvent& event);
  void OnMarkdownFileSelected(wxCommandEvent& event);
  void OnMarkdownFileActivated(wxCommandEvent& event);
  void OnEditorStatusChanged(wxCommandEvent& event);
  void OnActiveDocumentChanged(wxCommandEvent& event);
  void OnDocumentChanged(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void OnActivate(wxActivateEvent& event);

  void UpdatePreview();
  wxString FormatValidationResults() const;
  bool CreateNewFile(const wxString& filePath);
  bool ExecuteMarkdownCommand(MarkdownCommand command,
                              const wxString& argument = wxString(),
                              const wxString& secondaryArgument = wxString());
  bool IsMarkdownCommandAllowed(MarkdownCommand command) const;
  wxString GetDefaultNewFileDirectory() const;
  wxString MakeMarkdownImagePath(const wxString& imagePath) const;
  void UpdateDocumentCommandState();
  void ApplyWindowSettings();
  void SaveWindowSettings();
  void RefreshRecentMenus();
  void AddRecentFile(const wxString& filePath);
  void AddRecentFolder(const wxString& folderPath);
  wxString FormatRecentMenuLabel(size_t index, const wxString& path) const;

  // Layout methods
  void CreateLayout();

  FileTreePanel* m_fileTreePanel;
  EditorNotebook* m_editorNotebook;
  PreviewPanel* m_previewPanel;
  wxSplitterWindow* m_splitter;
  wxSplitterWindow* m_editorPreviewSplitter;
  wxMenu* m_recentFilesMenu;
  wxMenu* m_recentFoldersMenu;
  wxArrayString m_recentFiles;
  wxArrayString m_recentFolders;
  SettingsManager m_settingsManager;

  wxDECLARE_EVENT_TABLE();
};

#endif  // MAIN_FRAME_H
