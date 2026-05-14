#include "GlanceApp.h"
#include "MainFrame.h"
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/msgdlg.h>

bool GlanceApp::OnInit()
{
    wxInitAllImageHandlers();

    m_mainFrame = new MainFrame();
    m_mainFrame->Show(true);

    if (argc > 1)
    {
        wxFileName startupPath(argv[1]);
        startupPath.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);

        const wxString startupFullPath = startupPath.GetFullPath();

        if (wxDir::Exists(startupFullPath))
        {
            m_mainFrame->OpenFolder(startupFullPath);
        }
        else if (wxFile::Exists(startupFullPath))
        {
            wxFileName parentFolder(startupFullPath);
            parentFolder.SetFullName("");
            m_mainFrame->OpenFolder(parentFolder.GetPath());
            m_mainFrame->OpenFile(startupFullPath);
        }
        else
        {
            wxMessageBox("Path does not exist: " + startupFullPath,
                         "Unable to Open Path",
                         wxOK | wxICON_ERROR);
        }
    }

    return true;
}

int GlanceApp::OnExit()
{
    return 0;
}
