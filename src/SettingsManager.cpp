#include "SettingsManager.h"

#include <wx/config.h>
#include <wx/dir.h>
#include <wx/filename.h>

#include "DocumentManager.h"

namespace {
constexpr long MaxRecentItems = 10;

wxString NormalizeExistingPath(const wxString& path) {
  wxFileName fileName(path);
  fileName.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
  return fileName.GetFullPath();
}
}  // namespace

WindowSettings SettingsManager::LoadWindowSettings() const {
  wxConfig config("Glance");
  WindowSettings settings;

  config.SetPath("/Window");
  long x = 0;
  long y = 0;
  long width = 0;
  long height = 0;
  settings.hasGeometry = config.Read("X", &x) && config.Read("Y", &y) &&
                         config.Read("Width", &width) &&
                         config.Read("Height", &height);

  if (settings.hasGeometry) {
    settings.position = wxPoint(static_cast<int>(x), static_cast<int>(y));
    settings.size = wxSize(static_cast<int>(width), static_cast<int>(height));
  }

  config.Read("Maximized", &settings.maximized, false);
  config.Read("FileTreeSash", &settings.fileTreeSash, settings.fileTreeSash);
  config.Read("PreviewSash", &settings.previewSash, settings.previewSash);

  return settings;
}

void SettingsManager::SaveWindowSettings(const WindowSettings& settings) const {
  wxConfig config("Glance");
  config.SetPath("/Window");
  config.Write("X", settings.position.x);
  config.Write("Y", settings.position.y);
  config.Write("Width", settings.size.GetWidth());
  config.Write("Height", settings.size.GetHeight());
  config.Write("Maximized", settings.maximized);
  config.Write("FileTreeSash", settings.fileTreeSash);
  config.Write("PreviewSash", settings.previewSash);
  config.Flush();
}

wxArrayString SettingsManager::LoadRecentFiles() const {
  wxArrayString files = LoadRecentItems("RecentFiles");
  for (int i = static_cast<int>(files.GetCount()) - 1; i >= 0; --i) {
    if (!wxFileName::FileExists(files[static_cast<size_t>(i)])) {
      files.RemoveAt(static_cast<size_t>(i));
    }
  }
  return files;
}

wxArrayString SettingsManager::LoadRecentFolders() const {
  wxArrayString folders = LoadRecentItems("RecentFolders");
  for (int i = static_cast<int>(folders.GetCount()) - 1; i >= 0; --i) {
    if (!wxDir::Exists(folders[static_cast<size_t>(i)])) {
      folders.RemoveAt(static_cast<size_t>(i));
    }
  }
  return folders;
}

void SettingsManager::AddRecentFile(const wxString& filePath) const {
  AddRecentItem("RecentFiles", DocumentManager::NormalizePath(filePath));
}

void SettingsManager::AddRecentFolder(const wxString& folderPath) const {
  AddRecentItem("RecentFolders", NormalizeExistingPath(folderPath));
}

void SettingsManager::ClearRecentItems() const {
  wxConfig config("Glance");
  config.DeleteGroup("RecentFiles");
  config.DeleteGroup("RecentFolders");
  config.Flush();
}

wxArrayString SettingsManager::LoadRecentItems(
    const wxString& groupName) const {
  wxConfig config("Glance");
  config.SetPath("/" + groupName);

  long count = 0;
  config.Read("Count", &count, 0);

  wxArrayString items;
  for (long i = 0; i < count && i < MaxRecentItems; ++i) {
    wxString item;
    if (config.Read(wxString::Format("Item%ld", i), &item) && !item.empty()) {
      items.Add(item);
    }
  }

  return items;
}

void SettingsManager::SaveRecentItems(const wxString& groupName,
                                      const wxArrayString& items) const {
  wxConfig config("Glance");
  config.DeleteGroup(groupName);
  config.SetPath("/" + groupName);
  config.Write("Count", static_cast<long>(items.GetCount()));

  for (size_t i = 0; i < items.GetCount(); ++i) {
    config.Write(wxString::Format("Item%zu", i), items[i]);
  }

  config.Flush();
}

void SettingsManager::AddRecentItem(const wxString& groupName,
                                    const wxString& path) const {
  if (path.empty()) {
    return;
  }

  wxArrayString items = LoadRecentItems(groupName);
  const int existingIndex = items.Index(path);
  if (existingIndex != wxNOT_FOUND) {
    items.RemoveAt(static_cast<size_t>(existingIndex));
  }

  items.Insert(path, 0);
  while (items.GetCount() > MaxRecentItems) {
    items.RemoveAt(items.GetCount() - 1);
  }

  SaveRecentItems(groupName, items);
}
