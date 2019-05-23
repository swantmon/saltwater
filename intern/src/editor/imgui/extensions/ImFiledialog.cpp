
// Based on: https://github.com/ocornut/imgui/issues/88

#include "editor/edit_precompiled.h"

#include "base/base_defines.h"

#ifdef PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <malloc.h>
#include <stdio.h>
#endif

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../imgui.h"

#include "ImFiledialog.h"

#ifdef _WIN32
namespace Edit
{
    CImFileFialog::CImFileFialog(const std::string& _rTitle, const std::regex& _rFilter, const std::string& _rRoot, int _Config)
        : m_Title(_rTitle)
        , m_Regex(_rFilter)
        , m_Config((EConfig)_Config)
    { 
        m_RootPath = _rRoot;
        m_CurrentPath = _rRoot;
    }

    // -----------------------------------------------------------------------------

    const std::vector<std::string>& CImFileFialog::GetSelectedFiles() const 
    { 
        return m_SelectedFiles; 
    }

    // -----------------------------------------------------------------------------

    void CImFileFialog::FillRoots()
    {
        DWORD Drives = GetLogicalDrives();

        for (TCHAR CurrentDrive = 'A'; CurrentDrive <= 'Z'; CurrentDrive++, Drives >>= 1)
        {
            if (!(Drives & 1)) continue;

            BOOL success = FALSE;
            TCHAR rootPath[4];
            TCHAR volumeName[MAX_PATH + 1];

            rootPath[0] = CurrentDrive;
            rootPath[1] = ':';
            rootPath[2] = '\\';
            rootPath[3] = '\0';

            success = GetVolumeInformation(rootPath, volumeName, MAX_PATH + 1, nullptr, nullptr, nullptr, nullptr, 0);

            if (!success) continue;

#ifdef UNICODE
            int needed;
            LPSTR str;

            needed = WideCharToMultiByte(CP_UTF8, 0, rootPath, -1, nullptr, 0, nullptr, nullptr);
            if (needed <= 0) continue;
            str = (LPSTR)_malloca(needed);
            WideCharToMultiByte(CP_UTF8, 0, rootPath, -1, str, needed, nullptr, nullptr);
            std::string root = str;
            _freea(str);

            needed = WideCharToMultiByte(CP_UTF8, 0, volumeName, -1, nullptr, 0, nullptr, nullptr);
            if (needed <= 0) continue;
            str = (LPSTR)_malloca(needed);
            WideCharToMultiByte(CP_UTF8, 0, volumeName, -1, str, needed, nullptr, nullptr);
            std::string label = root + " (" + str + ")";
            _freea(str);
#else
            std::string root = rootName;
            std::string label = root + " (" + volumeName + ")";
#endif
            m_Roots.push_back({ root, label });
        }
    }
#else
    void CImFileFialog::FillRoots()
    {
        m_Roots.push_back({ "/", "(root)" });
    }
    #endif

    // -----------------------------------------------------------------------------

    void CImFileFialog::ClearCache() 
    {
        m_IsCacheDirty = true;

        m_Roots.clear();
        m_Directories.clear();
        m_Files.clear();
    }

    // -----------------------------------------------------------------------------

    void CImFileFialog::Open()
    {
        m_ShouldOpen = true;

        ClearCache();

        m_SelectedFiles.clear();

        m_Sorter.m_SortModeName = SORT_DOWN;
        m_Sorter.m_SortModeSize = UNSORTED;
        m_Sorter.m_SortModeDate = UNSORTED;
    }

    // -----------------------------------------------------------------------------

    bool CImFileFialog::Draw() 
    {
        bool IsDone = false;

        if (m_ShouldOpen)
        {
            ImGui::OpenPopup(m_Title.c_str());

            m_ShouldOpen = false;
        }

        if (ImGui::BeginPopupModal(m_Title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
        {
            if (m_IsCacheDirty) 
            {
                FillRoots();

                m_SpaceInfo = std::filesystem::space(m_CurrentPath);

                for (auto& rCurrentItemInPath : std::filesystem::directory_iterator(m_CurrentPath)) 
                {
                    if (rCurrentItemInPath.is_directory()) 
                    {
                        m_Directories.push_back(rCurrentItemInPath.path().filename().string());
                    }
                    else 
                    {
                        try 
                        {
                            auto status = rCurrentItemInPath.status();
                            auto lastWrite = std::filesystem::last_write_time(rCurrentItemInPath);
                            auto timeSinceEpoch = lastWrite.time_since_epoch();
                            auto count = timeSinceEpoch.count();

                            std::time_t dateTime = count / 100000000;

                            const std::tm* converted = std::localtime(&dateTime);

                            std::ostringstream formatted;

                            formatted << std::put_time(converted, "%c");

                            if (std::regex_match(rCurrentItemInPath.path().filename().string(), m_Regex))
                            {
                                m_Files.push_back({ rCurrentItemInPath.path().filename().string(), std::filesystem::file_size(rCurrentItemInPath), formatted.str(), dateTime });
                            }
                            
                        }
                        catch (...) 
                        {
                        }
                    }
                }

                if (m_Sorter.m_SortModeName != UNSORTED || m_Sorter.m_SortModeSize != UNSORTED || m_Sorter.m_SortModeDate != UNSORTED) 
                {
                    std::sort(m_Files.begin(), m_Files.end(), m_Sorter);
                }

                std::sort(m_Directories.begin(), m_Directories.end());
                m_IsCacheDirty = false;
            }

            bool DoGoHome = false;
            bool DoGoUp = false;

            std::string GoDownString = "";

            SFile* pSelectedFile = nullptr;

            if (ImGui::Button("Home")) DoGoHome = true;

            ImGui::SameLine();

            ImGui::Text(m_CurrentPath.string().c_str());

            ImGui::BeginChild("Directories", ImVec2(250, 350), true, ImGuiWindowFlags_HorizontalScrollbar);

            if (ImGui::TreeNode("Roots")) 
            {
                for (auto& p : m_Roots) 
                {
                    if (ImGui::Selectable(p.m_Label.c_str(), false, 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) 
                    {
                        GoDownString = p.m_Root;
                    }
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Directories", ImGuiTreeNodeFlags_DefaultOpen)) 
            {
                if (ImGui::Selectable("..", false, 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) 
                {
                    DoGoUp = true;
                }

                for (auto& p : m_Directories) 
                {
                    if (ImGui::Selectable(p.c_str(), false, 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) 
                    {
                        GoDownString = p;
                    }
                }

                ImGui::TreePop();
            }
            ImGui::EndChild();
   
            ImGui::SameLine();
            {
                std::string Header;

                ImGui::BeginChild("Files", ImVec2(500, 350), true, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Columns(3);

                switch (m_Sorter.m_SortModeName) 
                {
                case UNSORTED:
                    Header = "  File";
                    break;
                case SORT_DOWN:
                    Header = "v File";
                    break;
                case SORT_UP:
                    Header = "^ File";
                    break;
                }

                if (ImGui::Selectable(Header.c_str())) 
                {
                    switch (m_Sorter.m_SortModeName) 
                    {
                    case UNSORTED:
                        m_Sorter.m_SortModeName = SORT_DOWN;
                        break;
                    case SORT_DOWN:
                        m_Sorter.m_SortModeName = SORT_UP;
                        break;
                    case SORT_UP:
                        m_Sorter.m_SortModeName = UNSORTED;
                        break;
                    }

                    m_Sorter.m_SortModeSize = UNSORTED;
                    m_Sorter.m_SortModeDate = UNSORTED;

                    ClearCache();
                }

                ImGui::NextColumn();

                switch (m_Sorter.m_SortModeSize) 
                {
                case UNSORTED:
                    Header = "  Size";
                    break;
                case SORT_DOWN:
                    Header = "v Size";
                    break;
                case SORT_UP:
                    Header = "^ Size";
                    break;
                }

                if (ImGui::Selectable(Header.c_str())) 
                {
                    switch (m_Sorter.m_SortModeSize) 
                    {
                    case UNSORTED:
                        m_Sorter.m_SortModeSize = SORT_DOWN;
                        break;
                    case SORT_DOWN:
                        m_Sorter.m_SortModeSize = SORT_UP;
                        break;
                    case SORT_UP:
                        m_Sorter.m_SortModeSize = UNSORTED;
                        break;
                    }
                    m_Sorter.m_SortModeName = UNSORTED;
                    m_Sorter.m_SortModeDate = UNSORTED;
                    ClearCache();
                }
                ImGui::NextColumn();

                switch (m_Sorter.m_SortModeDate) 
                {
                case UNSORTED:
                    Header = "  Date & Time";
                    break;
                case SORT_DOWN:
                    Header = "v Date & Time";
                    break;
                case SORT_UP:
                    Header = "^ Date & Time";
                    break;
                }

                if (ImGui::Selectable(Header.c_str()))
                {
                    switch (m_Sorter.m_SortModeDate) 
                    {
                    case UNSORTED:
                        m_Sorter.m_SortModeDate = SORT_DOWN;
                        break;
                    case SORT_DOWN:
                        m_Sorter.m_SortModeDate = SORT_UP;
                        break;
                    case SORT_UP:
                        m_Sorter.m_SortModeDate = UNSORTED;
                        break;
                    }
                    m_Sorter.m_SortModeName = UNSORTED;
                    m_Sorter.m_SortModeSize = UNSORTED;

                    ClearCache();
                }

                ImGui::NextColumn();
                ImGui::Separator();

                for (auto& rCurrentFile : m_Files) 
{
                    std::string label = std::string("##") + rCurrentFile.m_Filename;

                    if (ImGui::Selectable(label.c_str(), rCurrentFile.m_Selected, ImGuiSelectableFlags_SpanAllColumns)) 
                    {
                        for (auto& rFile : m_Files) rFile.m_Selected = false;

                        rCurrentFile.m_Selected = true;
                    }

                    ImGui::SameLine();
                    ImGui::Text(rCurrentFile.m_Filename.c_str());
                    ImGui::NextColumn();
                    ImGui::Text(std::to_string(rCurrentFile.m_Size).c_str());
                    ImGui::NextColumn();
                    ImGui::Text(rCurrentFile.m_DateTime.c_str());
                    ImGui::NextColumn();

                    if (rCurrentFile.m_Selected) pSelectedFile = &rCurrentFile;
                }

                ImGui::EndChild();
            }

            std::string SelectedFileString;
            bool gotSelected = pSelectedFile;

            SelectedFileString = (m_CurrentPath / std::filesystem::path(pSelectedFile ? pSelectedFile->m_Filename : "...")).string();

            std::replace(SelectedFileString.begin(), SelectedFileString.end(), '\\', '/');

            ImGui::Text(SelectedFileString.c_str());

            if (!gotSelected) 
            {
                const ImVec4 lolight = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
                ImGui::PushStyleColor(ImGuiCol_Button, lolight);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, lolight);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, lolight);
            }

            if (ImGui::Button("OK", ImVec2(120, 30)) && gotSelected) 
            {
                m_SelectedFiles.clear();
                m_SelectedFiles.push_back(SelectedFileString);

                ImGui::CloseCurrentPopup();

                IsDone = true;
            }

            if (!gotSelected) ImGui::PopStyleColor(3);

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 30))) 
            {
                ImGui::CloseCurrentPopup();
                IsDone = true;
            }

            ImGui::EndPopup();

            if (DoGoUp) 
            {
                m_CurrentPath = m_CurrentPath.parent_path();

                ClearCache();
            }
            else if (!GoDownString.empty()) 
            {
                m_CurrentPath = m_CurrentPath / GoDownString;

                ClearCache();
            }
            else if (DoGoHome) 
            {
                ClearCache();
            }
        }

        return IsDone;
    }
} // namespace Edit
