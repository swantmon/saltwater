
#include "script/script_precompiled.h"

#include "gui/gui_event_handler.h"

#include "script/script_manager.h"

namespace Script
{
    CScriptManager::CScriptManager()
    {
    }

    // -----------------------------------------------------------------------------

    CScriptManager::~CScriptManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::OnStart()
    {
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CScriptManager::OnInputEvent));

        for (auto& rScript : m_Scripts)
        {
            rScript->Start();
        }
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::OnExit()
    {
        for (auto& rScript : m_Scripts)
        {
            rScript->Exit();
        }

        Gui::EventHandler::UnregisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CScriptManager::OnInputEvent));
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::Update()
    {
        for (auto& rScript : m_Scripts)
        {
            rScript->Update();
        }
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::OnPause()
    {
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::OnResume()
    {
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        for (auto& rScript : m_Scripts)
        {
            rScript->OnInput(_rInputEvent);
        }
    }
} // namespace Script