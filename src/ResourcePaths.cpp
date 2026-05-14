#include "ResourcePaths.h"
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <vector>

wxString GetResourcePath(const wxString& fileName)
{
    std::vector<wxString> resourceDirs;

#ifdef GLANCE_SOURCE_DIR
    resourceDirs.push_back(wxFileName(wxString::FromUTF8(GLANCE_SOURCE_DIR), "resources").GetFullPath());
#endif

    resourceDirs.push_back(wxFileName(wxGetCwd(), "resources").GetFullPath());

    wxFileName executablePath(wxStandardPaths::Get().GetExecutablePath());
    const wxString executableDir = executablePath.GetPath();
    resourceDirs.push_back(wxFileName(executableDir, "resources").GetFullPath());
    resourceDirs.push_back(wxFileName(executableDir + wxFILE_SEP_PATH + "..", "resources").GetFullPath());

    for (const wxString& resourceDir : resourceDirs)
    {
        wxFileName candidate(resourceDir, fileName);
        candidate.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
        if (candidate.FileExists())
        {
            return candidate.GetFullPath();
        }
    }

    return wxFileName("resources", fileName).GetFullPath();
}
