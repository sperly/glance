#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/init.h>

#include <iostream>
#include <vector>

#include "Document.h"
#include "DocumentManager.h"
#include "MarkdownRenderer.h"
#include "MarkdownValidator.h"

namespace {
int g_failureCount = 0;

std::string ToUtf8(const wxString& value) {
  return std::string(value.utf8_string());
}

void Expect(bool condition, const char* message) {
  if (condition) {
    return;
  }

  ++g_failureCount;
  std::cerr << "FAIL: " << message << "\n";
}

void ExpectContains(const wxString& value, const wxString& expected,
                    const char* message) {
  if (value.Contains(expected)) {
    return;
  }

  ++g_failureCount;
  std::cerr << "FAIL: " << message << "\n"
            << "Expected to contain: " << ToUtf8(expected) << "\n"
            << "Actual: " << ToUtf8(value) << "\n";
}

void ExpectNotContains(const wxString& value, const wxString& unexpected,
                       const char* message) {
  if (!value.Contains(unexpected)) {
    return;
  }

  ++g_failureCount;
  std::cerr << "FAIL: " << message << "\n"
            << "Did not expect: " << ToUtf8(unexpected) << "\n"
            << "Actual: " << ToUtf8(value) << "\n";
}

void TestDocumentSaveLoad() {
  const wxString filePath = wxFileName::CreateTempFileName("glance-doc-test");
  Expect(!filePath.empty(),
         "temp file path is created for document save/load test");
  if (filePath.empty()) {
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
  Expect(loaded.GetContent() == "# Title\n\nBody",
         "loaded document content matches saved content");
  Expect(!loaded.HasChangedOnDisk(),
         "freshly loaded document is current with disk");

  wxRemoveFile(filePath);
}

void TestDocumentSaveFailsForMissingFolder() {
  const wxString tempFile =
      wxFileName::CreateTempFileName("glance-missing-folder-test");
  Expect(!tempFile.empty(), "temp path is created for missing folder test");
  if (tempFile.empty()) {
    return;
  }
  wxRemoveFile(tempFile);

  wxFileName missingFile(tempFile + "-missing-dir", "note.md");
  Document document(missingFile.GetFullPath());
  document.SetContent("content");
  document.SetModified(true);

  wxString errorMessage;
  Expect(!document.Save(&errorMessage), "saving to a missing folder fails");
  Expect(document.IsModified(),
         "failed save leaves document marked as modified");
  ExpectContains(errorMessage, "Folder does not exist",
                 "failed save reports missing folder");
}

void TestDocumentManagerNormalizesDuplicatePaths() {
  const wxString filePath =
      wxFileName::CreateTempFileName("glance-manager-test");
  Expect(!filePath.empty(),
         "temp file path is created for document manager test");
  if (filePath.empty()) {
    return;
  }

  Document document(filePath);
  document.SetContent("content");
  document.SetModified(true);

  wxString errorMessage;
  Expect(document.Save(&errorMessage),
         "document manager fixture saves successfully");

  DocumentManager manager;
  Document* first = manager.OpenDocument(filePath, &errorMessage);
  Document* second =
      manager.OpenDocument(wxFileName(filePath).GetFullPath(), &errorMessage);

  Expect(first != nullptr, "first document open succeeds");
  Expect(first == second,
         "duplicate normalized path returns existing document");
  Expect(manager.GetDocuments().size() == 1,
         "duplicate normalized path is not opened twice");

  wxRemoveFile(filePath);
}

void TestDocumentManagerDoesNotKeepFailedOpen() {
  const wxString tempFile =
      wxFileName::CreateTempFileName("glance-open-missing-test");
  Expect(!tempFile.empty(), "temp path is created for failed open test");
  if (tempFile.empty()) {
    return;
  }
  wxRemoveFile(tempFile);

  DocumentManager manager;
  wxString errorMessage;
  Expect(
      manager.OpenDocument(tempFile + "-missing.md", &errorMessage) == nullptr,
      "opening a missing document fails");
  Expect(manager.GetDocuments().empty(), "failed open does not add a document");
}

void TestMarkdownRendererCoreFeatures() {
  MarkdownRenderer renderer;
  const wxString html = renderer.RenderDocument(
      "# Heading\n\n~~gone~~\n\n==marked==\n\n- [x] done\n");

  ExpectContains(html, "<h1>Heading</h1>", "heading renders as h1");
  ExpectContains(html, "class=\"glance-strike\"",
                 "strikethrough class is emitted");
  ExpectContains(html, "<mark>marked</mark>", "highlight renders as mark tag");
  ExpectContains(html, "type=\"checkbox\" disabled checked",
                 "checked task item renders as disabled checkbox");
}

void TestMarkdownRendererEscapesHtml() {
  MarkdownRenderer renderer;
  const wxString html =
      renderer.RenderDocument("<script>alert(\"x\") & more</script>");

  ExpectContains(
      html,
      "<p>&lt;script&gt;alert(&quot;x&quot;) &amp; more&lt;/script&gt;</p>",
      "raw HTML special characters are escaped");
}

void TestMarkdownRendererKeepsFormattingOutOfCodeSpans() {
  MarkdownRenderer renderer;
  const wxString html = renderer.RenderDocument("`**not bold**` and **bold**");

  ExpectContains(html, "<code>**not bold**</code>",
                 "inline code content is not formatted");
  ExpectContains(html, "<strong>bold</strong>",
                 "bold text outside inline code still formats");
  ExpectNotContains(html, "<code><strong>not bold</strong></code>",
                    "bold formatting is not applied inside inline code");
}

void TestMarkdownRendererRendersTablesWithInlineContent() {
  MarkdownRenderer renderer;
  const wxString html =
      renderer.RenderDocument("| A | B |\n| --- | :--- |\n| **x** | `y` |\n");

  ExpectContains(html,
                 "<table><thead><tr><th>A</th><th style=\"text-align: "
                 "left;\">B</th></tr></thead><tbody>",
                 "table header renders");
  ExpectContains(html, "<td><strong>x</strong></td>",
                 "table cell renders bold inline content");
  ExpectContains(html, "<td style=\"text-align: left;\"><code>y</code></td>",
                 "table cell renders code inline content");
}

void TestMarkdownRendererRendersTableAlignment() {
  MarkdownRenderer renderer;
  const wxString html = renderer.RenderDocument(
      "| Left | Center | Right |\n| :--- | :---: | ---: |\n| a | b | c |\n");

  ExpectContains(html, "<th style=\"text-align: left;\">Left</th>",
                 "left-aligned table header renders");
  ExpectContains(html, "<th style=\"text-align: center;\">Center</th>",
                 "center-aligned table header renders");
  ExpectContains(html, "<th style=\"text-align: right;\">Right</th>",
                 "right-aligned table header renders");
  ExpectContains(html, "<td style=\"text-align: left;\">a</td>",
                 "left-aligned table cell renders");
  ExpectContains(html, "<td style=\"text-align: center;\">b</td>",
                 "center-aligned table cell renders");
  ExpectContains(html, "<td style=\"text-align: right;\">c</td>",
                 "right-aligned table cell renders");
}

void TestMarkdownRendererClosesListWhenListTypeChanges() {
  MarkdownRenderer renderer;
  const wxString html =
      renderer.RenderDocument("- one\n- two\n\n1. three\n2. four\n");

  ExpectContains(html, "<ul>\n<li>one</li>\n<li>two</li>\n</ul>\n",
                 "unordered list is closed before next block");
  ExpectContains(html, "<ol>\n<li>three</li>\n<li>four</li>\n</ol>\n",
                 "ordered list renders after unordered list");
}

void TestMarkdownRendererResolvesNestedRelativeImagePaths() {
  const wxString sourceFilePath =
      wxFileName::CreateTempFileName("glance-renderer-test");
  Expect(!sourceFilePath.empty(),
         "temp source path is created for relative image test");
  if (sourceFilePath.empty()) {
    return;
  }

  MarkdownRenderer renderer;
  const wxString html = renderer.RenderDocument(
      "![Alt](assets/images/photo.png)", sourceFilePath);

  wxFileName expectedPath("assets/images/photo.png");
  expectedPath.MakeAbsolute(wxFileName(sourceFilePath).GetPath());
  expectedPath.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);

  ExpectContains(html, "<img alt=\"Alt\"", "relative image renders img tag");
  ExpectContains(html, "src=\"file://" + expectedPath.GetFullPath() + "\"",
                 "relative image path resolves against source file directory");

  wxRemoveFile(sourceFilePath);
}

void TestMarkdownRendererKeepsAbsoluteRemoteAndAnchorImagePaths() {
  const wxString sourceFilePath =
      wxFileName::CreateTempFileName("glance-renderer-path-test");
  Expect(!sourceFilePath.empty(),
         "temp source path is created for absolute image test");
  if (sourceFilePath.empty()) {
    return;
  }

  MarkdownRenderer renderer;
  const wxString html = renderer.RenderDocument(
      "![Remote](https://example.com/"
      "a.png)\n\n![Anchor](#icon)\n\n![Absolute](/tmp/a.png)",
      sourceFilePath);

  ExpectContains(html, "src=\"https://example.com/a.png\"",
                 "remote image path is preserved");
  ExpectContains(html, "src=\"#icon\"", "anchor image path is preserved");
  ExpectContains(html, "src=\"/tmp/a.png\"",
                 "absolute image path is preserved");
  ExpectNotContains(html, "src=\"file://https://example.com/a.png\"",
                    "remote image path is not converted to file URL");

  wxRemoveFile(sourceFilePath);
}

void TestMarkdownRendererUsesVanillaFlavor() {
  MarkdownRenderer renderer;
  const wxString html = renderer.RenderDocument(
      "~~gone~~\n\n==marked==\n\n- [x] done\n\n| A | B |\n| --- | --- |\n| 1 | "
      "2 |\n",
      wxString(), MarkdownFlavor::Vanilla);

  ExpectNotContains(html, "class=\"glance-strike\"",
                    "vanilla flavor does not render strikethrough extension");
  ExpectNotContains(html, "<mark>",
                    "vanilla flavor does not render highlight extension");
  ExpectNotContains(html, "type=\"checkbox\"",
                    "vanilla flavor does not render task list extension");
  ExpectNotContains(html, "<table>",
                    "vanilla flavor does not render table extension");
}

void TestMarkdownValidatorFlagsUnsupportedVanillaExtensions() {
  MarkdownValidator validator;
  const std::vector<MarkdownDiagnostic> diagnostics = validator.Validate(
      "~~gone~~\n\n==marked==\n\n- [x] done\n\n| A | B |\n| --- | --- |\n| 1 | "
      "2 |\n",
      MarkdownFlavor::Vanilla);

  Expect(diagnostics.size() == 4,
         "vanilla validator flags each unsupported GitHub extension");
}

void TestMarkdownValidatorAllowsGithubExtensions() {
  MarkdownValidator validator;
  const std::vector<MarkdownDiagnostic> diagnostics = validator.Validate(
      "~~gone~~\n\n- [x] done\n\n| A | B |\n| --- | --- |\n| 1 | 2 |\n",
      MarkdownFlavor::GitHub);

  Expect(diagnostics.empty(),
         "github validator allows supported GitHub extensions");
}

void TestMarkdownRendererRendersSubscriptAndSuperscript() {
  MarkdownRenderer renderer;
  const wxString html =
      renderer.RenderDocument("H~2~O and E = mc^2^ and ~~gone~~");

  ExpectContains(html, "H<sub>2</sub>O", "subscript renders as sub tag");
  ExpectContains(html, "mc<sup>2</sup>", "superscript renders as sup tag");
  ExpectContains(html, "class=\"glance-strike\"",
                 "strikethrough still renders separately from subscript");
}

void TestMarkdownRendererHandlesGithubLanguageFences() {
  MarkdownRenderer renderer;
  const wxString html =
      renderer.RenderDocument("```sh shell example\nprintf '<ok>'\n```\n");

  ExpectContains(html,
                 "<pre><code class=\"language-sh\">printf '&lt;ok&gt;'\n"
                 "</code></pre>",
                 "github language-labelled fenced code block renders");
  ExpectNotContains(html, "<p>```sh", "opening code fence is not rendered");
  ExpectNotContains(html, "<p>```</p>", "closing code fence is not rendered");
}

void TestMarkdownValidatorAllowsGithubLanguageFences() {
  MarkdownValidator validator;
  const std::vector<MarkdownDiagnostic> diagnostics = validator.Validate(
      "```cpp\nint main() {}\n```\n", MarkdownFlavor::GitHub);

  Expect(diagnostics.empty(),
         "github validator allows language-labelled fenced code blocks");
}

void TestMarkdownRendererUsesFlavorDefinedInlineTags() {
  MarkdownFlavorDefinition customFlavor{
      MarkdownFlavor::GitHub,
      "custom",
      "Custom Markdown",
      "Test-only custom Markdown flavor.",
      {{MarkdownTag::InlineCode, MarkdownTagKind::Inline, "Inline code",
        "@([^@]+)@", "<kbd>$1</kbd>", "", ""},
       {MarkdownTag::Bold, MarkdownTagKind::Inline, "Highlight",
        "==([^=]+)==", "<mark>$1</mark>", "", ""}}};

  MarkdownRenderer renderer;
  const wxString html = renderer.RenderDocument(
      "@shortcut@ and ==marked== and `plain`", wxString(), customFlavor);

  ExpectContains(html, "<kbd>shortcut</kbd>",
                 "custom inline code syntax renders with its template");
  ExpectContains(html, "<mark>marked</mark>",
                 "custom bold syntax renders with its template");
  ExpectContains(html, "`plain`",
                 "undefined regular inline code syntax is left as text");
  ExpectNotContains(html, "<code>plain</code>",
                    "custom flavor does not use missing inline code syntax");
}
}  // namespace

int main() {
  wxInitializer initializer;
  Expect(initializer.IsOk(), "wxWidgets initializes");
  if (!initializer.IsOk()) {
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
  TestMarkdownRendererRendersTableAlignment();
  TestMarkdownRendererClosesListWhenListTypeChanges();
  TestMarkdownRendererResolvesNestedRelativeImagePaths();
  TestMarkdownRendererKeepsAbsoluteRemoteAndAnchorImagePaths();
  TestMarkdownRendererUsesVanillaFlavor();
  TestMarkdownValidatorFlagsUnsupportedVanillaExtensions();
  TestMarkdownValidatorAllowsGithubExtensions();
  TestMarkdownRendererRendersSubscriptAndSuperscript();
  TestMarkdownRendererHandlesGithubLanguageFences();
  TestMarkdownValidatorAllowsGithubLanguageFences();
  TestMarkdownRendererUsesFlavorDefinedInlineTags();

  if (g_failureCount > 0) {
    std::cerr << g_failureCount << " test failure(s)\n";
    return 1;
  }

  return 0;
}
