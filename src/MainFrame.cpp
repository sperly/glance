#include "MainFrame.h"

#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/numdlg.h>
#include <wx/textdlg.h>

#include <vector>

#include "AboutDialog.h"
#include "Document.h"
#include "DocumentSettingsDialog.h"
#include "EditorNotebook.h"
#include "EmbeddedResources.h"
#include "FileTreePanel.h"
#include "HelpDialog.h"
#include "MarkdownFlavor.h"
#include "MarkdownValidator.h"
#include "PreviewPanel.h"

namespace {
enum {
  ID_OPEN_FOLDER = wxID_HIGHEST + 1,
  ID_NEW_FILE,
  ID_SAVE_ALL,
  ID_CLOSE_TAB,
  ID_FORMAT_BOLD,
  ID_FORMAT_ITALIC,
  ID_FORMAT_BOLD_ITALIC,
  ID_FORMAT_UNDERLINE,
  ID_FORMAT_STRIKETHROUGH,
  ID_FORMAT_SUBSCRIPT,
  ID_FORMAT_SUPERSCRIPT,
  ID_FORMAT_INLINE_CODE,
  ID_FORMAT_CODE_BLOCK,
  ID_FORMAT_BLOCKQUOTE,
  ID_FORMAT_HEADING_1,
  ID_FORMAT_HEADING_2,
  ID_FORMAT_HEADING_3,
  ID_FORMAT_HEADING_4,
  ID_FORMAT_HEADING_5,
  ID_FORMAT_HEADING_6,
  ID_FORMAT_BULLET_LIST,
  ID_FORMAT_NUMBERED_LIST,
  ID_FORMAT_TASK_LIST,
  ID_FORMAT_COMPLETED_TASK,
  ID_FORMAT_HORIZONTAL_RULE,
  ID_FORMAT_CLEAR_FORMATTING,
  ID_INSERT_LINK,
  ID_INSERT_IMAGE,
  ID_INSERT_TABLE,
  ID_INSERT_CODE_BLOCK,
  ID_INSERT_INLINE_CODE,
  ID_INSERT_BLOCKQUOTE,
  ID_INSERT_BULLET_LIST,
  ID_INSERT_NUMBERED_LIST,
  ID_INSERT_TASK_LIST,
  ID_INSERT_HORIZONTAL_RULE,
  ID_INSERT_DATE,
  ID_INSERT_TIME,
  ID_INSERT_DATE_TIME,
  ID_INSERT_HTML_COMMENT,
  ID_INSERT_FOOTNOTE,
  ID_INSERT_TOC,
  ID_DOCUMENT_SETTINGS,
  ID_DOCUMENT_VALIDATE,
  ID_HELP_SHOW_HELP,
  ID_HELP_SAVE_PREVIEW_HTML,
  ID_RECENT_FILE_BASE,
  ID_RECENT_FILE_LAST = ID_RECENT_FILE_BASE + 9,
  ID_RECENT_FOLDER_BASE,
  ID_RECENT_FOLDER_LAST = ID_RECENT_FOLDER_BASE + 9,
  ID_CLEAR_RECENT_ITEMS
};

bool IsMarkdownFilePath(const wxString& filePath) {
  const wxString extension = wxFileName(filePath).GetExt().Lower();
  return extension == "md" || extension == "markdown" || extension == "mdown" ||
         extension == "mkd";
}

wxString ToMarkdownPath(wxString path) {
  path.Replace("\\", "/");
  return path;
}

struct MarkdownMenuCommand {
  int menuId;
  MarkdownCommand command;
};

bool GetRequiredTagForMarkdownCommand(MarkdownCommand command,
                                      MarkdownTag* tag) {
  switch (command) {
    case MarkdownCommand::Bold:
      *tag = MarkdownTag::Bold;
      return true;
    case MarkdownCommand::Italic:
      *tag = MarkdownTag::Italic;
      return true;
    case MarkdownCommand::BoldItalic:
      *tag = MarkdownTag::BoldItalic;
      return true;
    case MarkdownCommand::Strikethrough:
      *tag = MarkdownTag::Strikethrough;
      return true;
    case MarkdownCommand::Subscript:
      *tag = MarkdownTag::Subscript;
      return true;
    case MarkdownCommand::Superscript:
      *tag = MarkdownTag::Superscript;
      return true;
    case MarkdownCommand::InlineCode:
      *tag = MarkdownTag::InlineCode;
      return true;
    case MarkdownCommand::CodeBlock:
      *tag = MarkdownTag::FencedCodeBlock;
      return true;
    case MarkdownCommand::Blockquote:
      *tag = MarkdownTag::Blockquote;
      return true;
    case MarkdownCommand::Heading1:
    case MarkdownCommand::Heading2:
    case MarkdownCommand::Heading3:
    case MarkdownCommand::Heading4:
    case MarkdownCommand::Heading5:
    case MarkdownCommand::Heading6:
      *tag = MarkdownTag::Heading;
      return true;
    case MarkdownCommand::BulletList:
      *tag = MarkdownTag::UnorderedList;
      return true;
    case MarkdownCommand::NumberedList:
      *tag = MarkdownTag::OrderedList;
      return true;
    case MarkdownCommand::TaskList:
    case MarkdownCommand::CompletedTask:
      *tag = MarkdownTag::TaskListItem;
      return true;
    case MarkdownCommand::HorizontalRule:
      *tag = MarkdownTag::HorizontalRule;
      return true;
    case MarkdownCommand::Link:
      *tag = MarkdownTag::Link;
      return true;
    case MarkdownCommand::Image:
      *tag = MarkdownTag::Image;
      return true;
    case MarkdownCommand::Table:
      *tag = MarkdownTag::Table;
      return true;
    default:
      return false;
  }
}

const std::vector<MarkdownMenuCommand>& GetMarkdownMenuCommands() {
  static const std::vector<MarkdownMenuCommand> commands = {
      {ID_FORMAT_BOLD, MarkdownCommand::Bold},
      {ID_FORMAT_ITALIC, MarkdownCommand::Italic},
      {ID_FORMAT_BOLD_ITALIC, MarkdownCommand::BoldItalic},
      {ID_FORMAT_STRIKETHROUGH, MarkdownCommand::Strikethrough},
      {ID_FORMAT_SUBSCRIPT, MarkdownCommand::Subscript},
      {ID_FORMAT_SUPERSCRIPT, MarkdownCommand::Superscript},
      {ID_FORMAT_INLINE_CODE, MarkdownCommand::InlineCode},
      {ID_FORMAT_CODE_BLOCK, MarkdownCommand::CodeBlock},
      {ID_FORMAT_BLOCKQUOTE, MarkdownCommand::Blockquote},
      {ID_FORMAT_HEADING_1, MarkdownCommand::Heading1},
      {ID_FORMAT_HEADING_2, MarkdownCommand::Heading2},
      {ID_FORMAT_HEADING_3, MarkdownCommand::Heading3},
      {ID_FORMAT_HEADING_4, MarkdownCommand::Heading4},
      {ID_FORMAT_HEADING_5, MarkdownCommand::Heading5},
      {ID_FORMAT_HEADING_6, MarkdownCommand::Heading6},
      {ID_FORMAT_BULLET_LIST, MarkdownCommand::BulletList},
      {ID_FORMAT_NUMBERED_LIST, MarkdownCommand::NumberedList},
      {ID_FORMAT_TASK_LIST, MarkdownCommand::TaskList},
      {ID_FORMAT_COMPLETED_TASK, MarkdownCommand::CompletedTask},
      {ID_FORMAT_HORIZONTAL_RULE, MarkdownCommand::HorizontalRule},
      {ID_INSERT_LINK, MarkdownCommand::Link},
      {ID_INSERT_IMAGE, MarkdownCommand::Image},
      {ID_INSERT_TABLE, MarkdownCommand::Table},
      {ID_INSERT_BULLET_LIST, MarkdownCommand::BulletList},
      {ID_INSERT_NUMBERED_LIST, MarkdownCommand::NumberedList},
      {ID_INSERT_TASK_LIST, MarkdownCommand::TaskList},
      {ID_INSERT_HORIZONTAL_RULE, MarkdownCommand::HorizontalRule},
  };

  return commands;
}
}  // namespace

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame) EVT_MENU(
    ID_NEW_FILE, MainFrame::OnFileNewFile) EVT_MENU(ID_OPEN_FOLDER,
                                                    MainFrame::OnFileOpenFolder)
    EVT_MENU(wxID_OPEN, MainFrame::OnFileOpenFile) EVT_MENU(
        wxID_SAVE,
        MainFrame::
            OnFileSave) EVT_MENU(ID_SAVE_ALL,
                                 MainFrame::
                                     OnFileSaveAll) EVT_MENU(ID_CLOSE_TAB,
                                                             MainFrame::
                                                                 OnFileCloseTab)
        EVT_MENU(wxID_PRINT, MainFrame::OnFilePrint) EVT_MENU(
            wxID_EXIT,
            MainFrame::OnFileExit) EVT_MENU_RANGE(ID_RECENT_FILE_BASE,
                                                  ID_RECENT_FILE_LAST,
                                                  MainFrame::OnRecentFile)
            EVT_MENU_RANGE(
                ID_RECENT_FOLDER_BASE, ID_RECENT_FOLDER_LAST,
                MainFrame::OnRecentFolder) EVT_MENU(ID_CLEAR_RECENT_ITEMS,
                                                    MainFrame::
                                                        OnClearRecentItems)
                EVT_MENU(wxID_UNDO, MainFrame::OnEditUndo) EVT_MENU(
                    wxID_REDO,
                    MainFrame::OnEditRedo) EVT_MENU(wxID_CUT,
                                                    MainFrame::OnEditCut)
                    EVT_MENU(wxID_COPY, MainFrame::OnEditCopy) EVT_MENU(
                        wxID_PASTE,
                        MainFrame::OnEditPaste) EVT_MENU(wxID_SELECTALL,
                                                         MainFrame::
                                                             OnEditSelectAll)
                        EVT_MENU_RANGE(ID_FORMAT_BOLD,
                                       ID_FORMAT_CLEAR_FORMATTING,
                                       MainFrame::OnFormatCommand)
                            EVT_MENU_RANGE(ID_INSERT_LINK, ID_INSERT_TOC,
                                           MainFrame::OnInsertCommand)
                                EVT_MENU(ID_DOCUMENT_SETTINGS,
                                         MainFrame::OnDocumentSettings)
                                    EVT_MENU(ID_DOCUMENT_VALIDATE,
                                             MainFrame::OnDocumentValidate)
                                        EVT_MENU(ID_HELP_SHOW_HELP,
                                                 MainFrame::OnHelpShowHelp)
                                            EVT_MENU(ID_HELP_SAVE_PREVIEW_HTML,
                                                     MainFrame::
                                                         OnHelpSavePreviewHtml)
                                                EVT_MENU(wxID_ABOUT,
                                                         MainFrame::OnHelpAbout)
                                                    EVT_CLOSE(
                                                        MainFrame::OnClose)
                                                        EVT_ACTIVATE(
                                                            MainFrame::
                                                                OnActivate)
                                                            wxEND_EVENT_TABLE()

                                                                MainFrame::
                                                                    MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Glance Markdown Editor", wxDefaultPosition,
              wxSize(1000, 700)),
      m_fileTreePanel(nullptr),
      m_editorNotebook(nullptr),
      m_previewPanel(nullptr),
      m_splitter(nullptr),
      m_editorPreviewSplitter(nullptr),
      m_recentFilesMenu(nullptr),
      m_recentFoldersMenu(nullptr) {
  m_recentFiles = m_settingsManager.LoadRecentFiles();
  m_recentFolders = m_settingsManager.LoadRecentFolders();

  wxMemoryInputStream iconStream(GetEmbeddedGlanceIconPngData(),
                                 GetEmbeddedGlanceIconPngSize());
  wxImage appIconImage(iconStream, wxBITMAP_TYPE_PNG);
  if (appIconImage.IsOk()) {
    wxIcon appIcon;
    appIcon.CopyFromBitmap(wxBitmap(appIconImage));
    SetIcon(appIcon);
  }

  CreateMenuBar();
  CreateLayout();
  CreateStatusBar(2);
  SetStatusText("Ready", 0);
  SetStatusText("Glance Markdown Editor", 1);
  ApplyWindowSettings();
  RefreshRecentMenus();
  UpdateDocumentCommandState();
}

MainFrame::~MainFrame() {}

void MainFrame::CreateMenuBar() {
  wxMenuBar* menuBar = new wxMenuBar();

  // File menu
  wxMenu* fileMenu = new wxMenu();
  fileMenu->Append(ID_NEW_FILE, "&New File...\tCtrl+N",
                   "Create a new Markdown file");
  fileMenu->Append(ID_OPEN_FOLDER, "Open &Folder...\tCtrl+Shift+O",
                   "Open a folder containing Markdown files");
  fileMenu->Append(wxID_OPEN, "&Open File...\tCtrl+O", "Open a Markdown file");
  m_recentFoldersMenu = new wxMenu();
  fileMenu->AppendSubMenu(m_recentFoldersMenu, "Recent F&olders");
  m_recentFilesMenu = new wxMenu();
  fileMenu->AppendSubMenu(m_recentFilesMenu, "Recent Fi&les");
  fileMenu->Append(ID_CLEAR_RECENT_ITEMS, "Clear Recent &Items",
                   "Clear recent files and folders");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_SAVE, "&Save File\tCtrl+S", "Save the current file");
  fileMenu->Append(ID_SAVE_ALL, "Save &All\tCtrl+Shift+S",
                   "Save all modified files");
  fileMenu->AppendSeparator();
  fileMenu->Append(ID_CLOSE_TAB, "&Close Tab\tCtrl+W",
                   "Close the current editor tab");
  fileMenu->Append(wxID_PRINT, "&Print...\tCtrl+P",
                   "Print the current document");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, "E&xit\tCtrl+Q", "Exit the application");

  // Edit menu
  wxMenu* editMenu = new wxMenu();
  editMenu->Append(wxID_UNDO, "&Undo\tCtrl+Z", "Undo the last edit");
  editMenu->Append(wxID_REDO, "&Redo\tCtrl+Y", "Redo the last undone edit");
  editMenu->AppendSeparator();
  editMenu->Append(wxID_CUT, "Cu&t\tCtrl+X", "Cut selected text");
  editMenu->Append(wxID_COPY, "&Copy\tCtrl+C", "Copy selected text");
  editMenu->Append(wxID_PASTE, "&Paste\tCtrl+V", "Paste text");
  editMenu->AppendSeparator();
  editMenu->Append(wxID_SELECTALL, "Select &All\tCtrl+A", "Select all text");

  // Format menu
  wxMenu* formatMenu = new wxMenu();
  formatMenu->Append(ID_FORMAT_BOLD, "&Bold\tCtrl+B",
                     "Wrap selection with bold Markdown");
  formatMenu->Append(ID_FORMAT_ITALIC, "&Italic\tCtrl+I",
                     "Wrap selection with italic Markdown");
  formatMenu->Append(ID_FORMAT_BOLD_ITALIC, "Bold &Italic",
                     "Wrap selection with bold italic Markdown");
  formatMenu->Append(ID_FORMAT_STRIKETHROUGH, "&Strikethrough",
                     "Wrap selection with strikethrough Markdown");
  formatMenu->Append(ID_FORMAT_SUBSCRIPT, "S&ubscript",
                     "Wrap selection with subscript Markdown");
  formatMenu->Append(ID_FORMAT_SUPERSCRIPT, "Su&perscript",
                     "Wrap selection with superscript Markdown");
  formatMenu->Append(ID_FORMAT_INLINE_CODE, "Inline &Code\tCtrl+`",
                     "Wrap selection with inline code markers");
  formatMenu->Append(ID_FORMAT_CODE_BLOCK, "Code &Block",
                     "Insert a fenced code block");
  formatMenu->AppendSeparator();
  formatMenu->Append(ID_FORMAT_BLOCKQUOTE, "Block&quote",
                     "Prefix selected lines as a blockquote");

  wxMenu* headingMenu = new wxMenu();
  headingMenu->Append(ID_FORMAT_HEADING_1, "Heading &1\tCtrl+1");
  headingMenu->Append(ID_FORMAT_HEADING_2, "Heading &2\tCtrl+2");
  headingMenu->Append(ID_FORMAT_HEADING_3, "Heading &3\tCtrl+3");
  headingMenu->Append(ID_FORMAT_HEADING_4, "Heading &4");
  headingMenu->Append(ID_FORMAT_HEADING_5, "Heading &5");
  headingMenu->Append(ID_FORMAT_HEADING_6, "Heading &6");
  formatMenu->AppendSubMenu(headingMenu, "&Heading");

  formatMenu->AppendSeparator();
  formatMenu->Append(ID_FORMAT_BULLET_LIST, "&Bullet List");
  formatMenu->Append(ID_FORMAT_NUMBERED_LIST, "&Numbered List");
  formatMenu->Append(ID_FORMAT_TASK_LIST, "&Task List");
  formatMenu->Append(ID_FORMAT_COMPLETED_TASK, "Completed Tas&k");
  formatMenu->AppendSeparator();
  formatMenu->Append(ID_FORMAT_HORIZONTAL_RULE, "Horizontal &Rule");
  formatMenu->Append(ID_FORMAT_CLEAR_FORMATTING, "Clear &Formatting");

  // Insert menu
  wxMenu* insertMenu = new wxMenu();
  insertMenu->Append(ID_INSERT_LINK, "&Link\tCtrl+K", "Insert a Markdown link");
  insertMenu->Append(ID_INSERT_IMAGE, "&Image...", "Insert a Markdown image");
  insertMenu->Append(ID_INSERT_TABLE, "&Table", "Insert a Markdown table");
  insertMenu->AppendSeparator();
  insertMenu->Append(ID_INSERT_BULLET_LIST, "&Bullet List");
  insertMenu->Append(ID_INSERT_NUMBERED_LIST, "&Numbered List");
  insertMenu->Append(ID_INSERT_TASK_LIST, "Task &List");
  insertMenu->Append(ID_INSERT_HORIZONTAL_RULE, "Horizontal &Rule");
  insertMenu->AppendSeparator();
  insertMenu->Append(ID_INSERT_DATE, "&Date");
  insertMenu->Append(ID_INSERT_TIME, "T&ime");
  insertMenu->Append(ID_INSERT_DATE_TIME, "Date and Ti&me");

  // Document menu
  wxMenu* documentMenu = new wxMenu();
  documentMenu->Append(ID_DOCUMENT_SETTINGS, "&Settings...",
                       "Choose settings for the current document");
  documentMenu->Append(ID_DOCUMENT_VALIDATE, "&Validate Markdown",
                       "Validate the current document against its Markdown "
                       "flavor");

  // Help menu
  wxMenu* helpMenu = new wxMenu();
  helpMenu->Append(ID_HELP_SHOW_HELP, "&Help\tF1",
                   "Open the Glance help guide");
  helpMenu->Append(ID_HELP_SAVE_PREVIEW_HTML, "Save Preview &HTML...",
                   "Save the rendered preview HTML to a file");
  helpMenu->AppendSeparator();
  helpMenu->Append(wxID_ABOUT, "&About...", "About this application");

  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(formatMenu, "F&ormat");
  menuBar->Append(insertMenu, "&Insert");
  menuBar->Append(documentMenu, "&Document");
  menuBar->Append(helpMenu, "&Help");

  SetMenuBar(menuBar);
}

void MainFrame::CreateLayout() {
  // Create the splitter window
  m_splitter = new wxSplitterWindow(this, wxID_ANY);
  m_splitter->SetMinimumPaneSize(160);
  m_splitter->SetSashGravity(0.2);

  // Create the file tree panel
  m_fileTreePanel = new FileTreePanel(m_splitter);
  m_fileTreePanel->Bind(wxEVT_GLANCE_FILE_SELECTED,
                        &MainFrame::OnMarkdownFileSelected, this);
  m_fileTreePanel->Bind(wxEVT_GLANCE_FILE_ACTIVATED,
                        &MainFrame::OnMarkdownFileActivated, this);

  m_editorPreviewSplitter = new wxSplitterWindow(m_splitter, wxID_ANY);
  m_editorPreviewSplitter->SetMinimumPaneSize(220);
  m_editorPreviewSplitter->SetSashGravity(0.625);

  m_editorNotebook = new EditorNotebook(m_editorPreviewSplitter);
  m_editorNotebook->Bind(wxEVT_GLANCE_EDITOR_STATUS_CHANGED,
                         &MainFrame::OnEditorStatusChanged, this);
  m_editorNotebook->Bind(wxEVT_GLANCE_ACTIVE_DOCUMENT_CHANGED,
                         &MainFrame::OnActiveDocumentChanged, this);
  m_editorNotebook->Bind(wxEVT_GLANCE_DOCUMENT_CHANGED,
                         &MainFrame::OnDocumentChanged, this);

  m_previewPanel = new PreviewPanel(m_editorPreviewSplitter);

  m_editorPreviewSplitter->SplitVertically(m_editorNotebook, m_previewPanel,
                                           500);

  // Split the window
  m_splitter->SplitVertically(m_fileTreePanel, m_editorPreviewSplitter, 220);

  // Set the sizer for the main frame
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(m_splitter, 1, wxEXPAND);
  SetSizer(mainSizer);
}

bool MainFrame::OpenFolder(const wxString& folderPath) {
  if (m_editorNotebook->HasModifiedDocuments() &&
      !m_editorNotebook->ConfirmCloseAll()) {
    SetStatusText("Open folder cancelled", 0);
    return false;
  }

  if (m_fileTreePanel->OpenFolder(folderPath)) {
    m_editorNotebook->CloseAllWithoutPrompt();
    m_previewPanel->Clear();
    SetStatusText("Opened folder: " + m_fileTreePanel->GetCurrentFolder(), 0);
    AddRecentFolder(m_fileTreePanel->GetCurrentFolder());
    return true;
  }

  wxMessageBox("Unable to open folder:\n" + folderPath, "Open Folder Failed",
               wxOK | wxICON_ERROR, this);
  SetStatusText("Failed to open folder: " + folderPath, 0);
  return false;
}

bool MainFrame::OpenFile(const wxString& filePath) {
  if (!IsMarkdownFilePath(filePath)) {
    wxMessageBox("Only Markdown files can be opened.", "Unsupported File",
                 wxOK | wxICON_WARNING, this);
    return false;
  }

  wxString errorMessage;
  Document* document = m_editorNotebook->OpenFile(filePath, &errorMessage);
  if (!document) {
    wxMessageBox(errorMessage, "Open Failed", wxOK | wxICON_ERROR, this);
    SetStatusText("Failed to open file: " + filePath, 0);
    return false;
  }

  document->SetMarkdownFlavor(
      m_settingsManager.LoadDocumentMarkdownFlavor(filePath));
  SetStatusText("Opened file: " + filePath, 0);
  AddRecentFile(filePath);
  UpdatePreview();
  return true;
}

void MainFrame::OnFileNewFile(wxCommandEvent& event) {
  wxFileDialog dialog(this, "New Markdown File", GetDefaultNewFileDirectory(),
                      "Untitled.md",
                      "Markdown files "
                      "(*.md;*.markdown;*.mdown;*.mkd)|*.md;*.markdown;*.mdown;"
                      "*.mkd|All files (*.*)|*.*",
                      wxFD_SAVE);

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  wxFileName fileName(dialog.GetPath());
  if (fileName.GetExt().empty()) {
    fileName.SetExt("md");
  }
  fileName.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);

  CreateNewFile(fileName.GetFullPath());
}

void MainFrame::OnFileOpenFolder(wxCommandEvent& event) {
  wxDirDialog dialog(this, "Select a folder containing Markdown files", "",
                     wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

  if (dialog.ShowModal() == wxID_OK) {
    OpenFolder(dialog.GetPath());
  }
}

void MainFrame::OnFileOpenFile(wxCommandEvent& event) {
  wxFileDialog dialog(this, "Open Markdown file", "", "",
                      "Markdown files "
                      "(*.md;*.markdown;*.mdown;*.mkd)|*.md;*.markdown;*.mdown;"
                      "*.mkd|All files (*.*)|*.*",
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (dialog.ShowModal() == wxID_OK) {
    OpenFile(dialog.GetPath());
  }
}

void MainFrame::OnFileSave(wxCommandEvent& event) {
  wxString errorMessage;
  if (!m_editorNotebook->SaveCurrent(&errorMessage)) {
    wxMessageBox(errorMessage, "Save Failed", wxOK | wxICON_ERROR, this);
    return;
  }

  SetStatusText("Saved current file", 0);
  UpdatePreview();
}

void MainFrame::OnFileSaveAll(wxCommandEvent& event) {
  wxString errorMessage;
  if (!m_editorNotebook->SaveAll(&errorMessage)) {
    wxMessageBox(errorMessage, "Save Failed", wxOK | wxICON_ERROR, this);
    return;
  }

  SetStatusText("Saved all files", 0);
  UpdatePreview();
}

void MainFrame::OnFileCloseTab(wxCommandEvent& event) {
  m_editorNotebook->CloseCurrent();
  UpdatePreview();
}

void MainFrame::OnFilePrint(wxCommandEvent& event) {
  Document* document = m_editorNotebook->GetCurrentDocument();
  if (!document) {
    wxMessageBox("No document is open.", "Print", wxOK | wxICON_INFORMATION,
                 this);
    return;
  }

  if (!m_previewPanel->PrintMarkdown(
          document->GetContent(), document->GetFilePath(),
          document->GetFileName(), document->GetMarkdownFlavor())) {
    SetStatusText("Print cancelled or failed", 0);
    return;
  }

  SetStatusText("Opened print dialog: " + document->GetFileName(), 0);
}

void MainFrame::OnFileExit(wxCommandEvent& event) { Close(true); }

void MainFrame::OnRecentFile(wxCommandEvent& event) {
  const int index = event.GetId() - ID_RECENT_FILE_BASE;
  if (index >= 0 && index < static_cast<int>(m_recentFiles.GetCount())) {
    OpenFile(m_recentFiles[static_cast<size_t>(index)]);
  }
}

void MainFrame::OnRecentFolder(wxCommandEvent& event) {
  const int index = event.GetId() - ID_RECENT_FOLDER_BASE;
  if (index >= 0 && index < static_cast<int>(m_recentFolders.GetCount())) {
    OpenFolder(m_recentFolders[static_cast<size_t>(index)]);
  }
}

void MainFrame::OnClearRecentItems(wxCommandEvent& event) {
  m_settingsManager.ClearRecentItems();
  m_recentFiles.Clear();
  m_recentFolders.Clear();
  RefreshRecentMenus();
  SetStatusText("Recent files and folders cleared", 0);
}

void MainFrame::OnEditUndo(wxCommandEvent& event) { m_editorNotebook->Undo(); }

void MainFrame::OnEditRedo(wxCommandEvent& event) { m_editorNotebook->Redo(); }

void MainFrame::OnEditCut(wxCommandEvent& event) { m_editorNotebook->Cut(); }

void MainFrame::OnEditCopy(wxCommandEvent& event) { m_editorNotebook->Copy(); }

void MainFrame::OnEditPaste(wxCommandEvent& event) {
  m_editorNotebook->Paste();
}

void MainFrame::OnEditSelectAll(wxCommandEvent& event) {
  m_editorNotebook->SelectAllText();
}

void MainFrame::OnFormatCommand(wxCommandEvent& event) {
  switch (event.GetId()) {
    case ID_FORMAT_BOLD:
      ExecuteMarkdownCommand(MarkdownCommand::Bold);
      break;
    case ID_FORMAT_ITALIC:
      ExecuteMarkdownCommand(MarkdownCommand::Italic);
      break;
    case ID_FORMAT_BOLD_ITALIC:
      ExecuteMarkdownCommand(MarkdownCommand::BoldItalic);
      break;
    case ID_FORMAT_UNDERLINE:
      ExecuteMarkdownCommand(MarkdownCommand::Underline);
      break;
    case ID_FORMAT_STRIKETHROUGH:
      ExecuteMarkdownCommand(MarkdownCommand::Strikethrough);
      break;
    case ID_FORMAT_SUBSCRIPT:
      ExecuteMarkdownCommand(MarkdownCommand::Subscript);
      break;
    case ID_FORMAT_SUPERSCRIPT:
      ExecuteMarkdownCommand(MarkdownCommand::Superscript);
      break;
    case ID_FORMAT_INLINE_CODE:
      ExecuteMarkdownCommand(MarkdownCommand::InlineCode);
      break;
    case ID_FORMAT_CODE_BLOCK:
      ExecuteMarkdownCommand(MarkdownCommand::CodeBlock);
      break;
    case ID_FORMAT_BLOCKQUOTE:
      ExecuteMarkdownCommand(MarkdownCommand::Blockquote);
      break;
    case ID_FORMAT_HEADING_1:
      ExecuteMarkdownCommand(MarkdownCommand::Heading1);
      break;
    case ID_FORMAT_HEADING_2:
      ExecuteMarkdownCommand(MarkdownCommand::Heading2);
      break;
    case ID_FORMAT_HEADING_3:
      ExecuteMarkdownCommand(MarkdownCommand::Heading3);
      break;
    case ID_FORMAT_HEADING_4:
      ExecuteMarkdownCommand(MarkdownCommand::Heading4);
      break;
    case ID_FORMAT_HEADING_5:
      ExecuteMarkdownCommand(MarkdownCommand::Heading5);
      break;
    case ID_FORMAT_HEADING_6:
      ExecuteMarkdownCommand(MarkdownCommand::Heading6);
      break;
    case ID_FORMAT_BULLET_LIST:
      ExecuteMarkdownCommand(MarkdownCommand::BulletList);
      break;
    case ID_FORMAT_NUMBERED_LIST:
      ExecuteMarkdownCommand(MarkdownCommand::NumberedList);
      break;
    case ID_FORMAT_TASK_LIST:
      ExecuteMarkdownCommand(MarkdownCommand::TaskList);
      break;
    case ID_FORMAT_COMPLETED_TASK:
      ExecuteMarkdownCommand(MarkdownCommand::CompletedTask);
      break;
    case ID_FORMAT_HORIZONTAL_RULE:
      ExecuteMarkdownCommand(MarkdownCommand::HorizontalRule);
      break;
    case ID_FORMAT_CLEAR_FORMATTING:
      ExecuteMarkdownCommand(MarkdownCommand::ClearFormatting);
      break;
  }
}

void MainFrame::OnInsertCommand(wxCommandEvent& event) {
  switch (event.GetId()) {
    case ID_INSERT_LINK: {
      wxDialog dialog(this, wxID_ANY, "Insert Link");
      wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

      mainSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Link text:"), 0,
                     wxLEFT | wxRIGHT | wxTOP, 12);
      wxTextCtrl* textCtrl = new wxTextCtrl(&dialog, wxID_ANY, "link text");
      mainSizer->Add(textCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 12);

      mainSizer->Add(new wxStaticText(&dialog, wxID_ANY, "Link URL:"), 0,
                     wxLEFT | wxRIGHT | wxTOP, 12);
      wxTextCtrl* urlCtrl =
          new wxTextCtrl(&dialog, wxID_ANY, "https://example.com");
      mainSizer->Add(urlCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 12);

      wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
      buttonSizer->AddButton(new wxButton(&dialog, wxID_OK));
      buttonSizer->AddButton(new wxButton(&dialog, wxID_CANCEL));
      buttonSizer->Realize();
      mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 12);

      dialog.SetSizerAndFit(mainSizer);
      textCtrl->SetFocus();
      textCtrl->SelectAll();

      if (dialog.ShowModal() == wxID_OK) {
        ExecuteMarkdownCommand(MarkdownCommand::Link, urlCtrl->GetValue(),
                               textCtrl->GetValue());
      }
      break;
    }
    case ID_INSERT_IMAGE: {
      wxFileDialog fileDialog(
          this, "Insert Image", "", "",
          "Image files "
          "(*.png;*.jpg;*.jpeg;*.gif;*.svg;*.webp)|*.png;*.jpg;*.jpeg;*.gif;*."
          "svg;*.webp|All files (*.*)|*.*",
          wxFD_OPEN | wxFD_FILE_MUST_EXIST);
      if (fileDialog.ShowModal() != wxID_OK) {
        break;
      }

      wxFileName imageName(fileDialog.GetPath());
      wxTextEntryDialog altDialog(this, "Alt text:", "Insert Image",
                                  imageName.GetName());
      if (altDialog.ShowModal() == wxID_OK) {
        ExecuteMarkdownCommand(MarkdownCommand::Image,
                               MakeMarkdownImagePath(fileDialog.GetPath()),
                               altDialog.GetValue());
      }
      break;
    }
    case ID_INSERT_TABLE: {
      wxNumberEntryDialog dialog(
          this, "Choose how many columns the Markdown table should have.",
          "Columns:", "Insert Table", 3, 1, 12);
      if (dialog.ShowModal() == wxID_OK) {
        ExecuteMarkdownCommand(MarkdownCommand::Table,
                               wxString::Format("%ld", dialog.GetValue()));
      }
      break;
    }
    case ID_INSERT_CODE_BLOCK:
      ExecuteMarkdownCommand(MarkdownCommand::CodeBlock);
      break;
    case ID_INSERT_INLINE_CODE:
      ExecuteMarkdownCommand(MarkdownCommand::InlineCode);
      break;
    case ID_INSERT_BLOCKQUOTE:
      ExecuteMarkdownCommand(MarkdownCommand::Blockquote);
      break;
    case ID_INSERT_BULLET_LIST:
      ExecuteMarkdownCommand(MarkdownCommand::BulletList);
      break;
    case ID_INSERT_NUMBERED_LIST:
      ExecuteMarkdownCommand(MarkdownCommand::NumberedList);
      break;
    case ID_INSERT_TASK_LIST:
      ExecuteMarkdownCommand(MarkdownCommand::TaskList);
      break;
    case ID_INSERT_HORIZONTAL_RULE:
      ExecuteMarkdownCommand(MarkdownCommand::HorizontalRule);
      break;
    case ID_INSERT_DATE:
      ExecuteMarkdownCommand(MarkdownCommand::Date);
      break;
    case ID_INSERT_TIME:
      ExecuteMarkdownCommand(MarkdownCommand::Time);
      break;
    case ID_INSERT_DATE_TIME:
      ExecuteMarkdownCommand(MarkdownCommand::DateTime);
      break;
    case ID_INSERT_HTML_COMMENT:
      ExecuteMarkdownCommand(MarkdownCommand::HtmlComment);
      break;
    case ID_INSERT_FOOTNOTE:
      ExecuteMarkdownCommand(MarkdownCommand::Footnote);
      break;
    case ID_INSERT_TOC:
      ExecuteMarkdownCommand(MarkdownCommand::TocMarker);
      break;
  }
}

void MainFrame::OnDocumentSettings(wxCommandEvent& event) {
  Document* document = m_editorNotebook->GetCurrentDocument();
  if (!document) {
    wxMessageBox("No document is open.", "Document Settings",
                 wxOK | wxICON_INFORMATION, this);
    return;
  }

  DocumentSettingsDialog dialog(this, document->GetMarkdownFlavor());
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  const MarkdownFlavor selectedFlavor = dialog.GetSelectedFlavor();
  document->SetMarkdownFlavor(selectedFlavor);
  m_settingsManager.SaveDocumentMarkdownFlavor(document->GetFilePath(),
                                               selectedFlavor);
  SetStatusText(
      "Markdown flavor: " + MarkdownFlavorToDisplayName(selectedFlavor), 0);
  UpdateDocumentCommandState();
  UpdatePreview();
}

void MainFrame::OnDocumentValidate(wxCommandEvent& event) {
  Document* document = m_editorNotebook->GetCurrentDocument();
  if (!document) {
    wxMessageBox("No document is open.", "Validate Markdown",
                 wxOK | wxICON_INFORMATION, this);
    return;
  }

  wxMessageBox(FormatValidationResults(), "Validate Markdown", wxOK, this);
}

void MainFrame::OnHelpSavePreviewHtml(wxCommandEvent& event) {
  if (!m_editorNotebook->GetCurrentDocument()) {
    wxMessageBox("Open a Markdown document before saving preview HTML.",
                 "Save Preview HTML", wxOK | wxICON_INFORMATION, this);
    return;
  }

  wxFileName currentFile(m_editorNotebook->GetCurrentFilePath());
  wxString defaultName = currentFile.GetName() + ".html";

  wxFileDialog dialog(
      this, "Save Preview HTML", currentFile.GetPath(), defaultName,
      "HTML files (*.html;*.htm)|*.html;*.htm|All files (*.*)|*.*",
      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  wxFile file(dialog.GetPath(), wxFile::write);
  if (!file.IsOpened() ||
      !file.Write(m_previewPanel->GetHtmlSource(), wxConvUTF8)) {
    wxMessageBox("Unable to save preview HTML: " + dialog.GetPath(),
                 "Save Failed", wxOK | wxICON_ERROR, this);
    return;
  }

  SetStatusText("Saved preview HTML: " + dialog.GetPath(), 0);
}

void MainFrame::OnHelpShowHelp(wxCommandEvent& event) {
  HelpDialog dialog(this);
  dialog.ShowModal();
}

void MainFrame::OnHelpAbout(wxCommandEvent& event) {
  AboutDialog dialog(this);
  dialog.ShowModal();
}

void MainFrame::OnMarkdownFileSelected(wxCommandEvent& event) {
  SetStatusText("Selected file: " + event.GetString(), 0);
}

void MainFrame::OnMarkdownFileActivated(wxCommandEvent& event) {
  OpenFile(event.GetString());
}

void MainFrame::OnEditorStatusChanged(wxCommandEvent& event) {
  SetStatusText(event.GetString(), 1);
}

void MainFrame::OnActiveDocumentChanged(wxCommandEvent& event) {
  if (!event.GetString().empty()) {
    SetStatusText("Active file: " + event.GetString(), 0);
  }
  UpdateDocumentCommandState();
  UpdatePreview();
}

void MainFrame::OnDocumentChanged(wxCommandEvent& event) { UpdatePreview(); }

void MainFrame::OnClose(wxCloseEvent& event) {
  if (m_editorNotebook && !m_editorNotebook->ConfirmCloseAll()) {
    if (event.CanVeto()) {
      event.Veto();
    }
    return;
  }

  SaveWindowSettings();
  Destroy();
}

void MainFrame::OnActivate(wxActivateEvent& event) {
  if (event.GetActive() && m_editorNotebook &&
      m_editorNotebook->CheckForExternalChanges(this)) {
    UpdatePreview();
    SetStatusText("Reloaded externally changed file", 0);
  }

  event.Skip();
}

void MainFrame::UpdatePreview() {
  if (!m_previewPanel || !m_editorNotebook) {
    return;
  }

  if (!m_editorNotebook->GetCurrentDocument()) {
    m_previewPanel->Clear();
    return;
  }

  m_previewPanel->ShowMarkdown(
      m_editorNotebook->GetCurrentContent(),
      m_editorNotebook->GetCurrentFilePath(),
      m_editorNotebook->GetCurrentDocument()->GetMarkdownFlavor());
}

wxString MainFrame::FormatValidationResults() const {
  Document* document = m_editorNotebook->GetCurrentDocument();
  if (!document) {
    return "No document is open.";
  }

  MarkdownValidator validator;
  const std::vector<MarkdownDiagnostic> diagnostics = validator.Validate(
      m_editorNotebook->GetCurrentContent(), document->GetMarkdownFlavor());
  const wxString flavorName =
      MarkdownFlavorToDisplayName(document->GetMarkdownFlavor());

  if (diagnostics.empty()) {
    return "No validation issues found for " + flavorName + ".";
  }

  wxString message =
      wxString::Format("%zu validation issue(s) found for %s:\n\n",
                       diagnostics.size(), flavorName.c_str());
  const size_t maxShown = 25;
  for (size_t i = 0; i < diagnostics.size() && i < maxShown; ++i) {
    const MarkdownDiagnostic& diagnostic = diagnostics[i];
    const wxString severity =
        diagnostic.severity == MarkdownDiagnosticSeverity::Error ? "Error"
                                                                 : "Warning";
    message += wxString::Format("Line %zu: %s: %s\n", diagnostic.line,
                                severity.c_str(), diagnostic.message.c_str());
  }

  if (diagnostics.size() > maxShown) {
    message += wxString::Format("\n%zu more issue(s) not shown.",
                                diagnostics.size() - maxShown);
  }

  return message;
}

bool MainFrame::CreateNewFile(const wxString& filePath) {
  if (!IsMarkdownFilePath(filePath)) {
    wxMessageBox("New files must use a Markdown file extension.",
                 "Unsupported File", wxOK | wxICON_WARNING, this);
    return false;
  }

  wxFileName fileName(filePath);
  if (!wxFileName::DirExists(fileName.GetPath())) {
    wxMessageBox("Folder does not exist:\n" + fileName.GetPath(),
                 "New File Failed", wxOK | wxICON_ERROR, this);
    return false;
  }

  if (fileName.FileExists()) {
    wxMessageDialog dialog(this,
                           "File already exists:\n" + filePath +
                               "\n\nOpen the existing file instead?",
                           "File Exists", wxYES_NO | wxICON_QUESTION);
    if (dialog.ShowModal() == wxID_YES) {
      return OpenFile(filePath);
    }

    return false;
  }

  wxFile file(filePath, wxFile::write);
  if (!file.IsOpened()) {
    wxMessageBox("Unable to create file:\n" + filePath, "New File Failed",
                 wxOK | wxICON_ERROR, this);
    return false;
  }

  file.Close();

  const wxString currentFolder =
      m_fileTreePanel ? m_fileTreePanel->GetCurrentFolder() : wxString();
  if (!currentFolder.empty()) {
    wxFileName relativeFile(filePath);
    if (relativeFile.MakeRelativeTo(currentFolder)) {
      m_fileTreePanel->RefreshTree();
    }
  }

  SetStatusText("Created file: " + filePath, 0);
  return OpenFile(filePath);
}

bool MainFrame::ExecuteMarkdownCommand(MarkdownCommand command,
                                       const wxString& argument,
                                       const wxString& secondaryArgument) {
  if (!IsMarkdownCommandAllowed(command)) {
    SetStatusText("Command is not available for this Markdown flavor", 0);
    return false;
  }

  if (!m_editorNotebook->ExecuteMarkdownCommand(command, argument,
                                                secondaryArgument)) {
    SetStatusText("No document is open", 0);
    return false;
  }

  UpdatePreview();
  return true;
}

bool MainFrame::IsMarkdownCommandAllowed(MarkdownCommand command) const {
  Document* document =
      m_editorNotebook ? m_editorNotebook->GetCurrentDocument() : nullptr;
  if (!document) {
    return false;
  }

  MarkdownTag requiredTag;
  if (!GetRequiredTagForMarkdownCommand(command, &requiredTag)) {
    return true;
  }

  const MarkdownFlavorDefinition& definition =
      GetMarkdownFlavorDefinition(document->GetMarkdownFlavor());
  return MarkdownFlavorHasTag(definition, requiredTag);
}

wxString MainFrame::GetDefaultNewFileDirectory() const {
  if (m_fileTreePanel && !m_fileTreePanel->GetCurrentFolder().empty()) {
    return m_fileTreePanel->GetCurrentFolder();
  }

  if (m_editorNotebook && !m_editorNotebook->GetCurrentFilePath().empty()) {
    return wxFileName(m_editorNotebook->GetCurrentFilePath()).GetPath();
  }

  return wxGetCwd();
}

wxString MainFrame::MakeMarkdownImagePath(const wxString& imagePath) const {
  wxFileName imageName(imagePath);
  imageName.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);

  wxString markdownPath = imageName.GetFullPath();
  const wxString currentFilePath =
      m_editorNotebook ? m_editorNotebook->GetCurrentFilePath() : wxString();
  if (!currentFilePath.empty()) {
    wxFileName currentFile(currentFilePath);
    wxFileName relativeImage(imageName);
    if (relativeImage.MakeRelativeTo(currentFile.GetPath())) {
      markdownPath = relativeImage.GetFullPath();
    }
  }

  return ToMarkdownPath(markdownPath);
}

void MainFrame::UpdateDocumentCommandState() {
  wxMenuBar* menuBar = GetMenuBar();
  if (!menuBar) {
    return;
  }

  const bool hasDocument =
      m_editorNotebook && m_editorNotebook->GetCurrentDocument();

  menuBar->EnableTop(1, hasDocument);  // Edit
  menuBar->EnableTop(2, hasDocument);  // Format
  menuBar->EnableTop(3, hasDocument);  // Insert
  menuBar->EnableTop(4, hasDocument);  // Document

  const int documentCommandIds[] = {
      wxID_SAVE,
      ID_SAVE_ALL,
      ID_CLOSE_TAB,
      wxID_PRINT,
      wxID_UNDO,
      wxID_REDO,
      wxID_CUT,
      wxID_COPY,
      wxID_PASTE,
      wxID_SELECTALL,
      ID_FORMAT_BOLD,
      ID_FORMAT_ITALIC,
      ID_FORMAT_BOLD_ITALIC,
      ID_FORMAT_STRIKETHROUGH,
      ID_FORMAT_SUBSCRIPT,
      ID_FORMAT_SUPERSCRIPT,
      ID_FORMAT_INLINE_CODE,
      ID_FORMAT_CODE_BLOCK,
      ID_FORMAT_BLOCKQUOTE,
      ID_FORMAT_HEADING_1,
      ID_FORMAT_HEADING_2,
      ID_FORMAT_HEADING_3,
      ID_FORMAT_HEADING_4,
      ID_FORMAT_HEADING_5,
      ID_FORMAT_HEADING_6,
      ID_FORMAT_BULLET_LIST,
      ID_FORMAT_NUMBERED_LIST,
      ID_FORMAT_TASK_LIST,
      ID_FORMAT_COMPLETED_TASK,
      ID_FORMAT_HORIZONTAL_RULE,
      ID_FORMAT_CLEAR_FORMATTING,
      ID_INSERT_LINK,
      ID_INSERT_IMAGE,
      ID_INSERT_TABLE,
      ID_INSERT_BULLET_LIST,
      ID_INSERT_NUMBERED_LIST,
      ID_INSERT_TASK_LIST,
      ID_INSERT_HORIZONTAL_RULE,
      ID_INSERT_DATE,
      ID_INSERT_TIME,
      ID_INSERT_DATE_TIME,
      ID_DOCUMENT_SETTINGS,
      ID_DOCUMENT_VALIDATE,
      ID_HELP_SAVE_PREVIEW_HTML,
  };

  for (int id : documentCommandIds) {
    menuBar->Enable(id, hasDocument);
  }

  for (const MarkdownMenuCommand& command : GetMarkdownMenuCommands()) {
    menuBar->Enable(command.menuId,
                    hasDocument && IsMarkdownCommandAllowed(command.command));
  }
}

void MainFrame::ApplyWindowSettings() {
  const WindowSettings settings = m_settingsManager.LoadWindowSettings();
  if (settings.hasGeometry) {
    SetSize(settings.position.x, settings.position.y, settings.size.GetWidth(),
            settings.size.GetHeight());
  }

  if (m_splitter && settings.fileTreeSash > 0) {
    m_splitter->SetSashPosition(settings.fileTreeSash);
  }

  if (m_editorPreviewSplitter && settings.previewSash > 0) {
    m_editorPreviewSplitter->SetSashPosition(settings.previewSash);
  }

  if (settings.maximized) {
    Maximize(true);
  }
}

void MainFrame::SaveWindowSettings() {
  WindowSettings settings;
  settings.maximized = IsMaximized();
  settings.position = GetPosition();
  settings.size = GetSize();
  settings.fileTreeSash =
      m_splitter ? m_splitter->GetSashPosition() : settings.fileTreeSash;
  settings.previewSash = m_editorPreviewSplitter
                             ? m_editorPreviewSplitter->GetSashPosition()
                             : settings.previewSash;
  m_settingsManager.SaveWindowSettings(settings);
}

void MainFrame::RefreshRecentMenus() {
  auto repopulateMenu = [this](wxMenu* menu, const wxArrayString& items,
                               int firstId, const wxString& emptyLabel) {
    if (!menu) {
      return;
    }

    while (menu->GetMenuItemCount() > 0) {
      wxMenuItem* item = menu->FindItemByPosition(0);
      menu->Delete(item);
    }

    if (items.IsEmpty()) {
      wxMenuItem* emptyItem = menu->Append(wxID_ANY, emptyLabel);
      emptyItem->Enable(false);
      return;
    }

    for (size_t i = 0; i < items.GetCount() && i < 10; ++i) {
      menu->Append(firstId + static_cast<int>(i),
                   FormatRecentMenuLabel(i, items[i]));
    }
  };

  repopulateMenu(m_recentFoldersMenu, m_recentFolders, ID_RECENT_FOLDER_BASE,
                 "(No recent folders)");
  repopulateMenu(m_recentFilesMenu, m_recentFiles, ID_RECENT_FILE_BASE,
                 "(No recent files)");

  if (wxMenuBar* menuBar = GetMenuBar()) {
    menuBar->Enable(ID_CLEAR_RECENT_ITEMS,
                    !m_recentFiles.IsEmpty() || !m_recentFolders.IsEmpty());
  }
}

void MainFrame::AddRecentFile(const wxString& filePath) {
  m_settingsManager.AddRecentFile(filePath);
  m_recentFiles = m_settingsManager.LoadRecentFiles();
  RefreshRecentMenus();
}

void MainFrame::AddRecentFolder(const wxString& folderPath) {
  m_settingsManager.AddRecentFolder(folderPath);
  m_recentFolders = m_settingsManager.LoadRecentFolders();
  RefreshRecentMenus();
}

wxString MainFrame::FormatRecentMenuLabel(size_t index,
                                          const wxString& path) const {
  wxString label = path;
  label.Replace("&", "&&");
  return wxString::Format("&%zu %s", index + 1, label);
}
