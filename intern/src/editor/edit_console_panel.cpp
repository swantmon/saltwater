
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_input_event.h"

#include "editor/edit_console_panel.h"

#include "editor/imgui/imgui.h"

#include "engine/core/core_console.h"
#include "engine/core/core_time.h"

#include "engine/graphic/gfx_main.h"
#include "engine/gui/gui_event_handler.h"

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
        : m_Input           ()
        , m_Items           ()
        , m_History         ()
        , m_PositonInHistory(-1)
        , m_ScrollToBottom  (true)
    {
        ClearLog();

        auto NewEntryDelegate = [&](Core::CConsole::EConsoleLevel _Level, const std::string _Entry)
        {
            std::string LogLevelString = Core::CConsole::GetInstance().GetLogLevelString(_Level);

            m_Items.push_back("[" + LogLevelString + "] " + _Entry);

            m_ScrollToBottom = true;
        };

        Core::CConsole::GetInstance().RegisterHandler(NewEntryDelegate);
    }

    // -----------------------------------------------------------------------------

    CConsolePanel::~CConsolePanel()
    {
        ClearLog();

        m_Items.clear();
        m_History.clear();
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
        if (ImGui::Button("Scroll to bottom")) m_ScrollToBottom = true;
        ImGui::SameLine();

        static ImGuiTextFilter TextFilter;
        TextFilter.Draw("Filter", 120);

        ImGui::Separator();

        // -----------------------------------------------------------------------------
        // Scrolling area
        // -----------------------------------------------------------------------------
        const float FooterHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -FooterHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);

        bool CopyToClipboard = false;

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();

            CopyToClipboard = ImGui::Selectable("Copy to clipboard");

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

            if (strstr(pItem, "[Error]"))
            {
                TextColor = ImColor(255, 40, 40);
            }
            else if (strstr(pItem, "[Warning]"))
            {
                TextColor = ImColor(255, 235, 40);
            }
            else if (strstr(pItem, "[Info]"))
            {
                TextColor = DefaultTextColor;
            }
            else if (strstr(pItem, "[Debug]"))
            {
                TextColor = ImColor(40, 180, 80);
            }
            else if (strncmp(pItem, "# ", 2) == 0)
            {
                TextColor = ImColor(100, 180, 255);
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
        ImGuiInputTextCallback TextEditCallback = [](ImGuiInputTextCallbackData* _pData)->int
        {
            CConsolePanel* pConsole = (CConsolePanel*)_pData->UserData;

            const auto& rHistory = pConsole->m_History;
            auto& rPositonInHistory = pConsole->m_PositonInHistory;

            switch (_pData->EventFlag)
            {
                case ImGuiInputTextFlags_CallbackHistory:
                {
                    // Example of HISTORY
                    const int prev_history_pos = rPositonInHistory;

                    if (_pData->EventKey == ImGuiKey_UpArrow)
                    {
                        if (rPositonInHistory == -1)    rPositonInHistory = rHistory.size() - 1;
                        else if (rPositonInHistory > 0) rPositonInHistory--;
                    }
                    else if (_pData->EventKey == ImGuiKey_DownArrow)
                    {
                        if (rPositonInHistory != -1)
                            if (++rPositonInHistory >= rHistory.size())
                                rPositonInHistory = -1;
                    }

                    if (prev_history_pos != rPositonInHistory)
                    {
                        const char* history_str = (rPositonInHistory >= 0) ? rHistory[rPositonInHistory].c_str() : "";

                        _pData->DeleteChars(0, _pData->BufTextLen);
                        _pData->InsertChars(0, history_str);
                    }
                }
            }
            return 0;
        };

        bool ReclaimFocus = false;

        char InputBuffer[255];

        strcpy_s(InputBuffer, m_Input.c_str());

        if (ImGui::InputText("##Input", InputBuffer, 255, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory, TextEditCallback, (void*)this))
        {
            std::string Input = InputBuffer;

            Helper::Trim(Input);

            if (Input.length() > 0) ExecuteCommand(Input);

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

    void CConsolePanel::ExecuteCommand(const std::string& _rCommand)
    {
        m_Items.push_back("# " + _rCommand);

        m_ScrollToBottom = true;

        // -----------------------------------------------------------------------------
        // Add to history
        // -----------------------------------------------------------------------------
        m_PositonInHistory = -1;

        m_History.push_back(_rCommand);

        // -----------------------------------------------------------------------------
        // Commands
        // -----------------------------------------------------------------------------
        if (_rCommand.compare("CLS") == 0)
        {
            ClearLog();
        }
        else if (_rCommand.compare("HISTORY") == 0)
        {
            int First = m_History.size() - 10;

            for (int Index = First > 0 ? First : 0; Index < m_History.size(); Index++)
            {
                m_Items.push_back(std::to_string(Index) + ": " + m_History[Index]);
            }
        }
        else
        {
            // -----------------------------------------------------------------------------
            // Send command via input event
            // -----------------------------------------------------------------------------
            Base::CInputEvent Event(Base::CInputEvent::Command, _rCommand);

            Gui::EventHandler::OnUserEvent(Event);
        }
    }
} // namespace GUI
} // namespace Edit