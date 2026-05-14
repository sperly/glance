#include "MainFrame.h"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_OPEN, MainFrame::OnFileOpenFolder)
    EVT_MENU(wxID_EXIT, MainFrame::OnFileExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnHelpAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Glance Markdown Editor", wxDefaultPosition, wxSize(1000, 700))
{
    CreateMenuBar();
    CreateStatusBar(2);
    SetStatusText("Ready", 0);
    SetStatusText("Glance Markdown Editor", 1);
}

MainFrame::~MainFrame()
{
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();
    
    // File menu
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_OPEN, "&Open Folder...\tCtrl+O", "Open a folder containing Markdown files");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tCtrl+Q", "Exit the application");
    
    // Help menu
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About...\tF1", "About this application");
    
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    
    SetMenuBar(menuBar);
}

void MainFrame::OnFileOpenFolder(wxCommandEvent& event)
{
    wxMessageBox("Open Folder functionality will be implemented in Phase 2", "Not Implemented", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnFileExit(wxCommandEvent& event)
{
    Close(true);
}

void MainFrame::OnHelpAbout(wxCommandEvent& event)
{
    wxMessageBox("Glance Markdown Editor\nA portable desktop application for editing Markdown files", "About Glance", wxOK | wxICON_INFORMATION);
}