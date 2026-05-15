#include "Document.h"
#include "DocumentManager.h"
#include "MarkdownRenderer.h"
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/init.h>
#include <cassert>

namespace
{
void TestDocumentSaveLoad()
{
    const wxString filePath = wxFileName::CreateTempFileName("glance-doc-test");
    assert(!filePath.empty());

    Document document(filePath);
    document.SetContent("# Title\n\nBody");
    document.SetModified(true);

    wxString errorMessage;
    assert(document.Save(&errorMessage));
    assert(!document.IsModified());

    Document loaded(filePath);
    assert(loaded.Load(&errorMessage));
    assert(loaded.GetContent() == "# Title\n\nBody");
    assert(!loaded.HasChangedOnDisk());

    wxRemoveFile(filePath);
}

void TestDocumentManagerNormalizesDuplicatePaths()
{
    const wxString filePath = wxFileName::CreateTempFileName("glance-manager-test");
    assert(!filePath.empty());

    Document document(filePath);
    document.SetContent("content");
    document.SetModified(true);

    wxString errorMessage;
    assert(document.Save(&errorMessage));

    DocumentManager manager;
    Document* first = manager.OpenDocument(filePath, &errorMessage);
    Document* second = manager.OpenDocument(wxFileName(filePath).GetFullPath(), &errorMessage);

    assert(first != nullptr);
    assert(first == second);
    assert(manager.GetDocuments().size() == 1);

    wxRemoveFile(filePath);
}

void TestMarkdownRendererCoreFeatures()
{
    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("# Heading\n\n~~gone~~\n\n- [x] done\n");

    assert(html.Contains("<h1>Heading</h1>"));
    assert(html.Contains("class=\"glance-strike\""));
    assert(html.Contains("type=\"checkbox\" disabled checked"));
}

void TestMarkdownRendererResolvesNestedRelativeImagePaths()
{
    const wxString sourceFilePath = wxFileName::CreateTempFileName("glance-renderer-test");
    assert(!sourceFilePath.empty());

    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("![Alt](assets/images/photo.png)", sourceFilePath);

    wxFileName expectedPath("assets/images/photo.png");
    expectedPath.MakeAbsolute(wxFileName(sourceFilePath).GetPath());
    expectedPath.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);

    assert(html.Contains("<img alt=\"Alt\""));
    assert(html.Contains("src=\"file://" + expectedPath.GetFullPath() + "\""));

    wxRemoveFile(sourceFilePath);
}
}

int main()
{
    wxInitializer initializer;
    assert(initializer.IsOk());

    TestDocumentSaveLoad();
    TestDocumentManagerNormalizesDuplicatePaths();
    TestMarkdownRendererCoreFeatures();
    TestMarkdownRendererResolvesNestedRelativeImagePaths();
    return 0;
}
