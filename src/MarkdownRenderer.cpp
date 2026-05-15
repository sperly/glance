#include "MarkdownRenderer.h"
#include <wx/filename.h>
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace
{
std::string ToStdString(const wxString& value)
{
    return std::string(value.utf8_string());
}

wxString ToWxString(const std::string& value)
{
    return wxString::FromUTF8(value);
}

std::string EscapeHtml(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (char ch : value)
    {
        switch (ch)
        {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        default:
            escaped += ch;
            break;
        }
    }

    return escaped;
}

std::string Trim(const std::string& value)
{
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();

    if (begin >= end)
    {
        return std::string();
    }

    return std::string(begin, end);
}

bool StartsWith(const std::string& value, const std::string& prefix)
{
    return value.rfind(prefix, 0) == 0;
}

std::vector<std::string> SplitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        lines.push_back(line);
    }

    if (!text.empty() && text.back() == '\n')
    {
        lines.emplace_back();
    }

    return lines;
}

std::vector<std::string> SplitTableRow(const std::string& line)
{
    std::string trimmed = Trim(line);
    if (!trimmed.empty() && trimmed.front() == '|')
    {
        trimmed.erase(trimmed.begin());
    }
    if (!trimmed.empty() && trimmed.back() == '|')
    {
        trimmed.pop_back();
    }

    std::vector<std::string> cells;
    std::stringstream stream(trimmed);
    std::string cell;
    while (std::getline(stream, cell, '|'))
    {
        cells.push_back(Trim(cell));
    }

    return cells;
}

bool IsTableSeparator(const std::string& line)
{
    const std::vector<std::string> cells = SplitTableRow(line);
    if (cells.empty())
    {
        return false;
    }

    return std::all_of(cells.begin(), cells.end(), [](const std::string& cell) {
        const std::string trimmed = Trim(cell);
        if (trimmed.size() < 3)
        {
            return false;
        }

        return std::all_of(trimmed.begin(), trimmed.end(), [](char ch) {
            return ch == '-' || ch == ':' || std::isspace(static_cast<unsigned char>(ch));
        });
    });
}

bool IsOrderedListItem(const std::string& line)
{
    static const std::regex orderedPattern("^\\s*\\d+\\.\\s+.+");
    return std::regex_match(line, orderedPattern);
}

bool IsUnorderedListItem(const std::string& line)
{
    static const std::regex unorderedPattern("^\\s*[-*+]\\s+.+");
    return std::regex_match(line, unorderedPattern);
}

std::string StripListMarker(const std::string& line)
{
    static const std::regex listMarker("^\\s*(?:[-*+]|\\d+\\.)\\s+");
    return std::regex_replace(line, listMarker, "");
}

bool IsTaskItem(const std::string& text, bool* checked)
{
    if (StartsWith(text, "[ ] "))
    {
        *checked = false;
        return true;
    }
    if (StartsWith(text, "[x] ") || StartsWith(text, "[X] "))
    {
        *checked = true;
        return true;
    }

    return false;
}

int HeadingLevel(const std::string& line)
{
    int level = 0;
    while (level < static_cast<int>(line.size()) && line[level] == '#')
    {
        ++level;
    }

    if (level == 0 || level > 6 || level >= static_cast<int>(line.size()) || line[level] != ' ')
    {
        return 0;
    }

    return level;
}

bool IsAbsoluteOrRemotePath(const std::string& path)
{
    return path.find("://") != std::string::npos || StartsWith(path, "#") || StartsWith(path, "/");
}

std::string ResolveImagePaths(const std::string& html, const wxString& baseDirectory)
{
    if (baseDirectory.empty())
    {
        return html;
    }

    static const std::regex imagePattern("<img alt=\"([^\"]*)\" src=\"([^\"]+)\">");
    std::string resolved;
    std::string::const_iterator searchStart = html.begin();
    std::smatch match;

    while (std::regex_search(searchStart, html.cend(), match, imagePattern))
    {
        resolved.append(searchStart, match[0].first);

        const std::string alt = match[1].str();
        const std::string source = match[2].str();
        if (IsAbsoluteOrRemotePath(source))
        {
            resolved += match[0].str();
        }
        else
        {
            wxFileName imagePath(ToWxString(source));
            imagePath.MakeAbsolute(baseDirectory);
            imagePath.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
            resolved += "<img alt=\"" + alt + "\" src=\"file://" + ToStdString(imagePath.GetFullPath()) + "\">";
        }

        searchStart = match[0].second;
    }

    resolved.append(searchStart, html.cend());
    return resolved;
}
}

wxString MarkdownRenderer::RenderDocument(const wxString& markdown, const wxString& sourceFilePath) const
{
    const std::vector<std::string> lines = SplitLines(ToStdString(markdown));
    wxString baseDirectory;
    if (!sourceFilePath.empty())
    {
        baseDirectory = wxFileName(sourceFilePath).GetPath();
    }

    std::string html;
    std::string paragraph;
    bool inCodeBlock = false;
    bool inUnorderedList = false;
    bool inOrderedList = false;

    auto closeParagraph = [&]() {
        if (!paragraph.empty())
        {
            html += "<p>" + ToStdString(RenderInline(ToWxString(paragraph), baseDirectory)) + "</p>\n";
            paragraph.clear();
        }
    };

    auto closeLists = [&]() {
        if (inUnorderedList)
        {
            html += "</ul>\n";
            inUnorderedList = false;
        }
        if (inOrderedList)
        {
            html += "</ol>\n";
            inOrderedList = false;
        }
    };

    for (size_t i = 0; i < lines.size(); ++i)
    {
        const std::string line = lines[i];
        const std::string trimmed = Trim(line);

        if (StartsWith(trimmed, "```"))
        {
            closeParagraph();
            closeLists();
            html += inCodeBlock ? "</code></pre>\n" : "<pre><code>";
            inCodeBlock = !inCodeBlock;
            continue;
        }

        if (inCodeBlock)
        {
            html += EscapeHtml(line) + "\n";
            continue;
        }

        if (trimmed.empty())
        {
            closeParagraph();
            closeLists();
            continue;
        }

        if (i + 1 < lines.size() && trimmed.find('|') != std::string::npos && IsTableSeparator(lines[i + 1]))
        {
            closeParagraph();
            closeLists();

            const std::vector<std::string> headers = SplitTableRow(trimmed);
            html += "<table><thead><tr>";
            for (const auto& header : headers)
            {
                html += "<th>" + ToStdString(RenderInline(ToWxString(header), baseDirectory)) + "</th>";
            }
            html += "</tr></thead><tbody>\n";

            i += 2;
            while (i < lines.size() && Trim(lines[i]).find('|') != std::string::npos)
            {
                html += "<tr>";
                for (const auto& cell : SplitTableRow(lines[i]))
                {
                    html += "<td>" + ToStdString(RenderInline(ToWxString(cell), baseDirectory)) + "</td>";
                }
                html += "</tr>\n";
                ++i;
            }
            --i;
            html += "</tbody></table>\n";
            continue;
        }

        const int headingLevel = HeadingLevel(trimmed);
        if (headingLevel > 0)
        {
            closeParagraph();
            closeLists();
            const std::string content = Trim(trimmed.substr(static_cast<size_t>(headingLevel)));
            html += "<h" + std::to_string(headingLevel) + ">" + ToStdString(RenderInline(ToWxString(content), baseDirectory)) +
                    "</h" + std::to_string(headingLevel) + ">\n";
            continue;
        }

        if (trimmed == "---" || trimmed == "***" || trimmed == "___")
        {
            closeParagraph();
            closeLists();
            html += "<hr>\n";
            continue;
        }

        if (StartsWith(trimmed, "> "))
        {
            closeParagraph();
            closeLists();
            html += "<blockquote><p>" + ToStdString(RenderInline(ToWxString(trimmed.substr(2)), baseDirectory)) + "</p></blockquote>\n";
            continue;
        }

        if (IsUnorderedListItem(line) || IsOrderedListItem(line))
        {
            closeParagraph();
            const bool ordered = IsOrderedListItem(line);
            if (ordered && !inOrderedList)
            {
                closeLists();
                html += "<ol>\n";
                inOrderedList = true;
            }
            else if (!ordered && !inUnorderedList)
            {
                closeLists();
                html += "<ul>\n";
                inUnorderedList = true;
            }

            std::string itemText = StripListMarker(line);
            bool checked = false;
            if (IsTaskItem(itemText, &checked))
            {
                itemText = itemText.substr(4);
                html += "<li class=\"task\"><input type=\"checkbox\" disabled";
                if (checked)
                {
                    html += " checked";
                }
                html += "> " + ToStdString(RenderInline(ToWxString(itemText), baseDirectory)) + "</li>\n";
            }
            else
            {
                html += "<li>" + ToStdString(RenderInline(ToWxString(itemText), baseDirectory)) + "</li>\n";
            }
            continue;
        }

        closeLists();
        if (!paragraph.empty())
        {
            paragraph += " ";
        }
        paragraph += trimmed;
    }

    closeParagraph();
    closeLists();
    if (inCodeBlock)
    {
        html += "</code></pre>\n";
    }

    return ToWxString(html);
}

wxString MarkdownRenderer::RenderInline(const wxString& text, const wxString& baseDirectory) const
{
    std::string html = EscapeHtml(ToStdString(text));

    html = std::regex_replace(html,
                              std::regex("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)"),
                              "<img alt=\"$1\" src=\"$2\">");
    html = ResolveImagePaths(html, baseDirectory);
    html = std::regex_replace(html,
                              std::regex("\\[([^\\]]+)\\]\\(([^\\)]+)\\)"),
                              "<a href=\"$2\">$1</a>");
    html = std::regex_replace(html, std::regex("`([^`]+)`"), "<code>$1</code>");
    html = std::regex_replace(html, std::regex("\\*\\*\\*([^*]+)\\*\\*\\*"), "<strong><em>$1</em></strong>");
    html = std::regex_replace(html, std::regex("\\*\\*([^*]+)\\*\\*"), "<strong>$1</strong>");
    html = std::regex_replace(html, std::regex("\\*([^*]+)\\*"), "<em>$1</em>");
    html = std::regex_replace(html, std::regex("~~([^~]+)~~"), "<span class=\"glance-strike\">$1</span>");

    return ToWxString(html);
}
