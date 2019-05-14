
#include "editor/edit_precompiled.h"

#include "engine/core/core_console.h"

#include "editor/edit_assets_panel.h"
#include "editor/edit_gui_factory.h"
#include "editor/edit_scene_graph_panel.h"

#include "engine/data/data_map.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"

#include "engine/graphic/gfx_highlight_renderer.h"

#include "imgui/imgui.h"

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

namespace Edit
{
namespace GUI
{
    CAssetsPanel::CAssetsPanel()
        : m_Title("Assets")
        , m_Regex(".*.[]?")
    {
        m_CurrentPath = std::filesystem::current_path();

        m_SelectedFiles.clear();

        m_Sorter.m_SortModeName = SORT_DOWN;
        m_Sorter.m_SortModeSize = UNSORTED;
        m_Sorter.m_SortModeDate = UNSORTED;

        RefreshCache();
    }

    // -----------------------------------------------------------------------------

    CAssetsPanel::~CAssetsPanel()
    {

    }

    // -----------------------------------------------------------------------------

    void CAssetsPanel::Render()
    {
        ImGui::SetNextWindowPos(ImVec2(30, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        ImGui::Begin("Assets", &m_IsVisible);

        bool DoGoUp = false;

        std::string GoDownString = "";

        SFile* pSelectedFile = nullptr;

        ImGui::SameLine();

        ImGui::Text(m_CurrentPath.string().c_str());

        ImGui::BeginChild("Directories", ImVec2(250, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

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

        ImGui::EndChild();

        ImGui::SameLine();

        std::string Header;

        ImGui::BeginChild("Files", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

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

            RefreshCache();
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
            RefreshCache();
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

            RefreshCache();
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

        std::string SelectedFileString;
        bool gotSelected = pSelectedFile;

        SelectedFileString = (m_CurrentPath / std::filesystem::path(pSelectedFile ? pSelectedFile->m_Filename : "...")).string();

        std::replace(SelectedFileString.begin(), SelectedFileString.end(), '\\', '/');
      
        if (DoGoUp)
        {
            m_CurrentPath = m_CurrentPath.parent_path();

            RefreshCache();
        }
        else if (!GoDownString.empty())
        {
            m_CurrentPath = m_CurrentPath / GoDownString;

            RefreshCache();
        }

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CAssetsPanel::GetName()
    {
        return m_Title.c_str();
    }

    // -----------------------------------------------------------------------------

    void CAssetsPanel::RefreshCache()
    {
        m_Roots.clear();
        m_Directories.clear();
        m_Files.clear();

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
    }
} // namespace GUI
} // namespace Edit