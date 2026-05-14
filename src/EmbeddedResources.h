#ifndef EMBEDDED_RESOURCES_H
#define EMBEDDED_RESOURCES_H

#include <cstddef>

const char* GetEmbeddedHelpMarkdown();
const unsigned char* GetEmbeddedGlanceIconPngData();
std::size_t GetEmbeddedGlanceIconPngSize();
const unsigned char* GetEmbeddedGlanceLogoPngData();
std::size_t GetEmbeddedGlanceLogoPngSize();

#endif // EMBEDDED_RESOURCES_H
