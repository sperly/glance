#ifndef EMBEDDED_RESOURCES_H
#define EMBEDDED_RESOURCES_H

#include <cstddef>

const char* GetEmbeddedHelpMarkdown();
const unsigned char* GetEmbeddedHighlightJsData();
std::size_t GetEmbeddedHighlightJsSize();
const unsigned char* GetEmbeddedHighlightCssData();
std::size_t GetEmbeddedHighlightCssSize();
const unsigned char* GetEmbeddedGlanceIconPngData();
std::size_t GetEmbeddedGlanceIconPngSize();
const unsigned char* GetEmbeddedGlanceLogoPngData();
std::size_t GetEmbeddedGlanceLogoPngSize();

#endif  // EMBEDDED_RESOURCES_H
