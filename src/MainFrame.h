#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include <wx/wx.h>

class MainFrame : public wxFrame
{
public:
    MainFrame();
    virtual ~MainFrame();

private:
    void CreateMenuBar();
    
    // Menu event handlers
    void OnFileOpenFolder(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);
    void OnHelpAbout(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

#endif // MAIN_FRAME_H