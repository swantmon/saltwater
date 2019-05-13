
#pragma once

#include "base/base_singleton.h"

#include "editor/edit_panel_interface.h"

#include "engine/data/data_entity.h"

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

namespace Edit
{
namespace GUI
{
    class CAssetsPanel : public IPanel
    {
        BASE_SINGLETON_FUNC(CAssetsPanel)

    public:

        CAssetsPanel();
       ~CAssetsPanel();

    public:

        void Render() override;

        const char* GetName() override;

    private:

        enum ESortMode
        {
            UNSORTED,
            SORT_DOWN,
            SORT_UP
        };

    private:

        struct SRoot
        {
            std::string m_Root;
            std::string m_Label;
        };

        struct SFile
        {
            std::string m_Filename;
            std::uintmax_t m_Size;
            std::string m_DateTime;
            std::time_t m_DateTimeTimeT;
            bool m_Selected;
        };

        struct SSorter
        {
            bool operator()(const SFile& a, const SFile& b) const
            {
                switch (m_SortModeName)
                {
                case SORT_DOWN: return a.m_Filename < b.m_Filename;
                case SORT_UP:   return a.m_Filename > b.m_Filename;
                }

                switch (m_SortModeSize)
                {
                case SORT_DOWN: return a.m_Size < b.m_Size;
                case SORT_UP:   return a.m_Size > b.m_Size;
                }

                switch (m_SortModeDate)
                {
                case SORT_DOWN: return a.m_DateTimeTimeT < b.m_DateTimeTimeT;
                case SORT_UP:   return a.m_DateTimeTimeT > b.m_DateTimeTimeT;
                }

                return false;
            }
            ESortMode m_SortModeName = SORT_DOWN;
            ESortMode m_SortModeSize = UNSORTED;
            ESortMode m_SortModeDate = UNSORTED;
        };

    private:

        std::string m_Title;
        std::regex m_Regex;

        std::vector<SRoot> m_Roots;
        std::vector<std::string> m_Directories;
        std::vector<SFile> m_Files;
        std::vector<std::string> m_SelectedFiles;

        SSorter m_Sorter;

        std::filesystem::path m_CurrentPath;
        std::filesystem::space_info m_SpaceInfo;

    private:

        void FillRoots();
        void ClearCache();
    };
} // namespace GUI
} // namespace Edit