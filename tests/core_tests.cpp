#include "Document.h"
#include "DocumentManager.h"
#include "MarkdownRenderer.h"
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/init.h>
#include <iostream>

namespace
{
int g_failureCount = 0;

std::string ToUtf8(const wxString& value)
{
    return std::string(value.utf8_string());
}

void Expect(bool condition, const char* message)
{
    if (condition)
    {
        return;
    }

    ++g_failureCount;
    std::cerr << "FAIL: " << message << "\n";
}

void ExpectContains(const wxString& value, const wxString& expected, const char* message)
{
    if (value.Contains(expected))
    {
        return;
    }

    ++g_failureCount;
    std::cerr << "FAIL: " << message << "\n"
              << "Expected to contain: " << ToUtf8(expected) << "\n"
              << "Actual: " << ToUtf8(value) << "\n";
}

void ExpectNotContains(const wxString& value, const wxString& unexpected, const char* message)
{
    if (!value.Contains(unexpected))
    {
        return;
    }

    ++g_failureCount;
    std::cerr << "FAIL: " << message << "\n"
              << "Did not expect: " << ToUtf8(unexpected) << "\n"
              << "Actual: " << ToUtf8(value) << "\n";
}

void TestDocumentSaveLoad()
{
    const wxString filePath = wxFileName::CreateTempFileName("glance-doc-test");
    Expect(!filePath.empty(), "temp file path is created for document save/load test");
    if (filePath.empty())
    {
        return;
    }

    Document document(filePath);
    document.SetContent("# Title\n\nBody");
    document.SetModified(true);

    wxString errorMessage;
    Expect(document.Save(&errorMessage), "document saves successfully");
    Expect(!document.IsModified(), "successful save clears modified state");

    Document loaded(filePath);
    Expect(loaded.Load(&errorMessage), "saved document loads successfully");
    Expect(loaded.GetContent() == "# Title\n\nBody", "loaded document content matches saved content");
    Expect(!loaded.HasChangedOnDisk(), "freshly loaded document is current with disk");

    wxRemoveFile(filePath);
}

void TestDocumentSaveFailsForMissingFolder()
{
    const wxString tempFile = wxFileName::CreateTempFileName("glance-missing-folder-test");
    Expect(!tempFile.empty(), "temp path is created for missing folder test");
    if (tempFile.empty())
    {
        return;
    }
    wxRemoveFile(tempFile);

    wxFileName missingFile(tempFile + "-missing-dir", "note.md");
    Document document(missingFile.GetFullPath());
    document.SetContent("content");
    document.SetModified(true);

    wxString errorMessage;
    Expect(!document.Save(&errorMessage), "saving to a missing folder fails");
    Expect(document.IsModified(), "failed save leaves document marked as modified");
    ExpectContains(errorMessage, "Folder does not exist", "failed save reports missing folder");
}

void TestDocumentManagerNormalizesDuplicatePaths()
{
    const wxString filePath = wxFileName::CreateTempFileName("glance-manager-test");
    Expect(!filePath.empty(), "temp file path is created for document manager test");
    if (filePath.empty())
    {
        return;
    }

    Document document(filePath);
    document.SetContent("content");
    document.SetModified(true);

    wxString errorMessage;
    Expect(document.Save(&errorMessage), "document manager fixture saves successfully");

    DocumentManager manager;
    Document* first = manager.OpenDocument(filePath, &errorMessage);
    Document* second = manager.OpenDocument(wxFileName(filePath).GetFullPath(), &errorMessage);

    Expect(first != nullptr, "first document open succeeds");
    Expect(first == second, "duplicate normalized path returns existing document");
    Expect(manager.GetDocuments().size() == 1, "duplicate normalized path is not opened twice");

    wxRemoveFile(filePath);
}

void TestDocumentManagerDoesNotKeepFailedOpen()
{
    const wxString tempFile = wxFileName::CreateTempFileName("glance-open-missing-test");
    Expect(!tempFile.empty(), "temp path is created for failed open test");
    if (tempFile.empty())
    {
        return;
    }
    wxRemoveFile(tempFile);

    DocumentManager manager;
    wxString errorMessage;
    Expect(manager.OpenDocument(tempFile + "-missing.md", &errorMessage) == nullptr,
           "opening a missing document fails");
    Expect(manager.GetDocuments().empty(), "failed open does not add a document");
}

void TestMarkdownRendererCoreFeatures()
{
    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("# Heading\n\n~~gone~~\n\n- [x] done\n");

    ExpectContains(html, "<h1>Heading</h1>", "heading renders as h1");
    ExpectContains(html, "class=\"glance-strike\"", "strikethrough class is emitted");
    ExpectContains(html, "type=\"checkbox\" disabled checked", "checked task item renders as disabled checkbox");
}

void TestMarkdownRendererEscapesHtml()
{
    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("<script>alert(\"x\") & more</script>");

    ExpectContains(html,
                   "<p>&lt;script&gt;alert(&quot;x&quot;) &amp; more&lt;/script&gt;</p>",
                   "raw HTML special characters are escaped");
}

void TestMarkdownRendererKeepsFormattingOutOfCodeSpans()
{
    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("`**not bold**` and **bold**");

    ExpectContains(html, "<code>**not bold**</code>", "inline code content is not formatted");
    ExpectContains(html, "<strong>bold</strong>", "bold text outside inline code still formats");
    ExpectNotContains(html, "<code><strong>not bold</strong></code>", "bold formatting is not applied inside inline code");
}

void TestMarkdownRendererRendersTablesWithInlineContent()
{
    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("| A | B |\n| --- | :--- |\n| **x** | `y` |\n");

    ExpectContains(html, "<table><thead><tr><th>A</th><th>B</th></tr></thead><tbody>",
                   "table header renders");
    ExpectContains(html, "<td><strong>x</strong></td>", "table cell renders bold inline content");
    ExpectContains(html, "<td><code>y</code></td>", "table cell renders code inline content");
}

void TestMarkdownRendererClosesListWhenListTypeChanges()
{
    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("- one\n- two\n\n1. three\n2. four\n");

    ExpectContains(html, "<ul>\n<li>one</li>\n<li>two</li>\n</ul>\n",
                   "unordered list is closed before next block");
    ExpectContains(html, "<ol>\n<li>three</li>\n<li>four</li>\n</ol>\n",
                   "ordered list renders after unordered list");
}

void TestMarkdownRendererResolvesNestedRelativeImagePaths()
{
    const wxString sourceFilePath = wxFileName::CreateTempFileName("glance-renderer-test");
    Expect(!sourceFilePath.empty(), "temp source path is created for relative image test");
    if (sourceFilePath.empty())
    {
        return;
    }

    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument("![Alt](assets/images/photo.png)", sourceFilePath);

    wxFileName expectedPath("assets/images/photo.png");
    expectedPath.MakeAbsolute(wxFileName(sourceFilePath).GetPath());
    expectedPath.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);

    ExpectContains(html, "<img alt=\"Alt\"", "relative image renders img tag");
    ExpectContains(html, "src=\"file://" + expectedPath.GetFullPath() + "\"",
                   "relative image path resolves against source file directory");

    wxRemoveFile(sourceFilePath);
}

void TestMarkdownRendererKeepsAbsoluteRemoteAndAnchorImagePaths()
{
    const wxString sourceFilePath = wxFileName::CreateTempFileName("glance-renderer-path-test");
    Expect(!sourceFilePath.empty(), "temp source path is created for absolute image test");
    if (sourceFilePath.empty())
    {
        return;
    }

    MarkdownRenderer renderer;
    const wxString html = renderer.RenderDocument(
        "![Remote](https://example.com/a.png)\n\n![Anchor](#icon)\n\n![Absolute](/tmp/a.png)",
        sourceFilePath);

    ExpectContains(html, "src=\"https://example.com/a.png\"", "remote image path is preserved");
    ExpectContains(html, "src=\"#icon\"", "anchor image path is preserved");
    ExpectContains(html, "src=\"/tmp/a.png\"", "absolute image path is preserved");
    ExpectNotContains(html, "src=\"file://https://example.com/a.png\"",
                      "remote image path is not converted to file URL");

    wxRemoveFile(sourceFilePath);
}
}

int main()
{
    wxInitializer initializer;
    Expect(initializer.IsOk(), "wxWidgets initializes");
    if (!initializer.IsOk())
    {
        return 1;
    }

    TestDocumentSaveLoad();
    TestDocumentSaveFailsForMissingFolder();
    TestDocumentManagerNormalizesDuplicatePaths();
    TestDocumentManagerDoesNotKeepFailedOpen();
    TestMarkdownRendererCoreFeatures();
    TestMarkdownRendererEscapesHtml();
    TestMarkdownRendererKeepsFormattingOutOfCodeSpans();
    TestMarkdownRendererRendersTablesWithInlineContent();
    TestMarkdownRendererClosesListWhenListTypeChanges();
    TestMarkdownRendererResolvesNestedRelativeImagePaths();
    TestMarkdownRendererKeepsAbsoluteRemoteAndAnchorImagePaths();

    if (g_failureCount > 0)
    {
        std::cerr << g_failureCount << " test failure(s)\n";
        return 1;
    }

    return 0;
}
