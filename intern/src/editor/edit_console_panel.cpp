
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor/edit_console_panel.h"

#include "editor/imgui/imgui.h"

#include "engine/core/core_time.h"

#include "engine/graphic/gfx_main.h"

#include <algorithm> 
#include <cctype>
#include <locale>

namespace Helper
{
    static inline void TrimLeft(std::string &_rString) 
    {
        _rString.erase(_rString.begin(), std::find_if(_rString.begin(), _rString.end(), [](int ch) 
        {
            return !std::isspace(ch);
        }));
    }

    static inline void TrimRight(std::string &_rString) 
    {
        _rString.erase(std::find_if(_rString.rbegin(), _rString.rend(), [](int ch) 
        {
            return !std::isspace(ch);
        }).base(), _rString.end());
    }

    static inline void Trim(std::string &_rString) 
    {
        TrimLeft(_rString);
        TrimRight(_rString);
    }
} // namespace Helper

namespace Edit
{
namespace GUI
{
    CConsolePanel::CConsolePanel()
    {
        ClearLog();

        AddLog("Welcome to Dear ImGui!");
    }

    // -----------------------------------------------------------------------------

    CConsolePanel::~CConsolePanel()
    {
        ClearLog();
    }

    // -----------------------------------------------------------------------------

    void CConsolePanel::Render()
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Console", &m_IsVisible))
        {
            ImGui::End();
            return;
        }

        // -----------------------------------------------------------------------------
        // Header
        // -----------------------------------------------------------------------------
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        static ImGuiTextFilter TextFilter;
        TextFilter.Draw("", 180);
        ImGui::PopStyleVar();

        ImGui::SameLine();

        if (ImGui::SmallButton("Clear")) ClearLog();
        ImGui::SameLine();

        bool CopyToClipboard = ImGui::SmallButton("Copy"); 
        ImGui::SameLine();

        if (ImGui::SmallButton("Scroll to bottom")) m_ScrollToBottom = true;

        ImGui::Separator();

        // -----------------------------------------------------------------------------
        // Scrolling area
        // -----------------------------------------------------------------------------
        const float FooterHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -FooterHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();

            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

        if (CopyToClipboard) ImGui::LogToClipboard();

        ImVec4 DefaultTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

        for (auto& rItem : m_Items)
        {
            const char* pItem = rItem.c_str();

            if (!TextFilter.PassFilter(pItem)) continue;

            // -----------------------------------------------------------------------------
            // Style
            // -----------------------------------------------------------------------------
            ImVec4 TextColor = DefaultTextColor;

            if (strstr(pItem, "[error]"))
            {
                TextColor = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
            }
            else if (strncmp(pItem, "# ", 2) == 0)
            {
                TextColor = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
            }

            ImGui::PushStyleColor(ImGuiCol_Text, TextColor);
            ImGui::TextUnformatted(pItem);
            ImGui::PopStyleColor();
        }

        if (CopyToClipboard) ImGui::LogFinish();

        if (m_ScrollToBottom) ImGui::SetScrollHereY(1.0f);

        m_ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::Separator();

        // -----------------------------------------------------------------------------
        // Input
        // -----------------------------------------------------------------------------
        ImGuiInputTextCallback TextEditCallback = [](ImGuiInputTextCallbackData* data)->int
        {
            CConsolePanel* pConsole = (CConsolePanel*)data->UserData;

            return 0;
        };

        bool ReclaimFocus = false;

        char InputBuffer[255];

        strcpy_s(InputBuffer, m_Input.c_str());

        if (ImGui::InputText("Input", InputBuffer, 255, ImGuiInputTextFlags_EnterReturnsTrue, TextEditCallback, (void*)this))
        {
            std::string Input = InputBuffer;

            Helper::Trim(Input);

            if (Input.length() > 0) ExecCommand(Input);

            ReclaimFocus = true;

            m_Input.clear();
        }

        // -----------------------------------------------------------------------------
        // Focus
        // -----------------------------------------------------------------------------
        ImGui::SetItemDefaultFocus();

        if (ReclaimFocus) ImGui::SetKeyboardFocusHere(-1);

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CConsolePanel::GetName()
    {
        return "Console";
    }

    // -----------------------------------------------------------------------------
    
    void CConsolePanel::ClearLog()
    {
        m_Items.clear();

        m_ScrollToBottom = true;
    }

    // -----------------------------------------------------------------------------

    void CConsolePanel::AddLog(const char* _pText, ...)
    {
        char Buffer[1024];

        va_list Args;
        va_start(Args, _pText);
        vsnprintf(Buffer, IM_ARRAYSIZE(Buffer), _pText, Args);
        Buffer[IM_ARRAYSIZE(Buffer) - 1] = 0;
        va_end(Args);

        m_Items.push_back(Buffer);

        m_ScrollToBottom = true;
    }

    // -----------------------------------------------------------------------------

    void CConsolePanel::ExecCommand(const std::string& _rCommand)
    {
        AddLog("# %s\n", _rCommand.c_str());
    }
} // namespace GUI
} // namespace Edit