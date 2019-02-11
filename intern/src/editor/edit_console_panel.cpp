
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_input_event.h"
#include "base/base_string_helper.h"

#include "editor/edit_console_panel.h"

#include "editor/imgui/imgui.h"

#include "engine/core/core_console.h"
#include "engine/core/core_time.h"

#include "engine/graphic/gfx_main.h"
#include "engine/gui/gui_event_handler.h"

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
            m_Items.push_back({ _Level, _Entry });

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
        ImGui::SetNextWindowPos(ImVec2(30, 510), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Console", &m_IsVisible))
        {
            ImGui::End();
            return;
        }

        // -----------------------------------------------------------------------------
        // Header
        // -----------------------------------------------------------------------------
        static ImGuiTextFilter TextFilter;
        TextFilter.Draw("Filter");

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

            m_ScrollToBottom = ImGui::Selectable("Scroll to bottom");

            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

        if (CopyToClipboard) ImGui::LogToClipboard();

        ImVec4 DefaultTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

        for (auto[Index, Message]  : m_Items)
        {
            if (!TextFilter.PassFilter(Message.c_str())) continue;

            ImVec4 TextColor = DefaultTextColor;

            switch (Index)
            {
            case Core::CConsole::EConsoleLevel::Error:
                TextColor = ImColor(255, 40, 40);
                Message = "[Error] " + Message;
                break;
            case Core::CConsole::EConsoleLevel::Warning:
                TextColor = ImColor(255, 235, 40);
                Message = "[Warning] " + Message;
                break;
            case Core::CConsole::EConsoleLevel::Info:
                TextColor = DefaultTextColor;
                Message = "[Info] " + Message;
                break;
            case Core::CConsole::EConsoleLevel::Debug:
                TextColor = ImColor(40, 180, 80);
                Message = "[Debug] " + Message;
                break;
            case -1:
                TextColor = ImColor(100, 180, 255);
                Message = "[#] " + Message;
                break;
            case -2:
                TextColor = DefaultTextColor;
                break;
            case Core::CConsole::EConsoleLevel::Default:
            default:
                TextColor = DefaultTextColor;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, TextColor);
            ImGui::TextUnformatted(Message.c_str());
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
                    const int PreviousHistoryPosition = rPositonInHistory;

                    if (_pData->EventKey == ImGuiKey_UpArrow)
                    {
                        if (rPositonInHistory == -1)    rPositonInHistory = static_cast<int>(rHistory.size() - 1);
                        else if (rPositonInHistory > 0) rPositonInHistory--;
                    }
                    else if (_pData->EventKey == ImGuiKey_DownArrow)
                    {
                        if (rPositonInHistory != -1)
                            if (++rPositonInHistory >= rHistory.size())
                                rPositonInHistory = -1;
                    }

                    if (PreviousHistoryPosition != rPositonInHistory)
                    {
                        const char* pHistoryText = (rPositonInHistory >= 0) ? rHistory[rPositonInHistory].c_str() : "";

                        _pData->DeleteChars(0, _pData->BufTextLen);
                        _pData->InsertChars(0, pHistoryText);
                    }
                }
            }
            return 0;
        };

        bool ReclaimFocus = false;

        char InputBuffer[255];

        strcpy_s(InputBuffer, m_Input.c_str());

        if (ImGui::InputText("Input", InputBuffer, 255, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory, TextEditCallback, (void*)this))
        {
            std::string Input = InputBuffer;

            Base::Trim(Input);

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
        m_Items.push_back({ -1, _rCommand });

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
            int First = glm::max(static_cast<int>(m_History.size()) - 10, 0);

            for (int Index = First; Index < m_History.size(); Index ++)
            {
                m_Items.push_back({ -2, std::to_string(Index) + ": " + m_History[Index] });
            }
        }
        else
        {
            // -----------------------------------------------------------------------------
            // Send command via input event
            // -----------------------------------------------------------------------------
            Base::CInputEvent Event(Base::CInputEvent::Command, _rCommand);

            Gui::EventHandler::OnEvent(Event);
        }
    }
} // namespace GUI
} // namespace Edit