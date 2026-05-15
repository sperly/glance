#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <wx/arrstr.h>
#include <wx/gdicmn.h>
#include <wx/string.h>

struct WindowSettings {
  bool hasGeometry = false;
  bool maximized = false;
  wxPoint position = wxDefaultPosition;
  wxSize size = wxSize(1000, 700);
  int fileTreeSash = 220;
  int previewSash = 500;
};

class SettingsManager {
 public:
  WindowSettings LoadWindowSettings() const;
  void SaveWindowSettings(const WindowSettings& settings) const;

  wxArrayString LoadRecentFiles() const;
  wxArrayString LoadRecentFolders() const;
  void AddRecentFile(const wxString& filePath) const;
  void AddRecentFolder(const wxString& folderPath) const;
  void ClearRecentItems() const;

 private:
  wxArrayString LoadRecentItems(const wxString& groupName) const;
  void SaveRecentItems(const wxString& groupName,
                       const wxArrayString& items) const;
  void AddRecentItem(const wxString& groupName, const wxString& path) const;
};

#endif  // SETTINGS_MANAGER_H
