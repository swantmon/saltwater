
#include "script/script_precompiled.h"

#include "data/data_script_component.h"
#include "data/data_component_manager.h"

#include "gui/gui_event_handler.h"

#include "script/script_manager.h"

namespace Script
{
    CScriptManager::CScriptManager()
        : m_CurrentID(0)
    {
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(BASE_DIRTY_COMPONENT_METHOD(&CScriptManager::OnDirtyComponent));
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
            if (!rScript->m_IsActive) continue;

            rScript->Start();

            rScript->m_IsStarted = true;
        }
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::OnExit()
    {
        for (auto& rScript : m_Scripts)
        {
            if (!rScript->m_IsActive) continue;

            rScript->Exit();
        }

        Gui::EventHandler::UnregisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CScriptManager::OnInputEvent));
    }

    // -----------------------------------------------------------------------------

    void CScriptManager::Update()
    {
        for (auto& rScript : m_Scripts)
        {
            if (!rScript->m_IsActive) continue;

            if (!rScript->m_IsStarted)
            {
                rScript->Start();

                rScript->m_IsStarted = true;
            }
            
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

    void CScriptManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        // -----------------------------------------------------------------------------
        // Only if component has changed
        // -----------------------------------------------------------------------------
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CScriptComponent>()) return;

        Dt::CScriptComponent* pScriptComponent = static_cast<Dt::CScriptComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pScriptComponent->GetDirtyFlags();

        if ((DirtyFlags & Dt::CScriptComponent::DirtyInfo) != 0)
        {
            assert(pScriptComponent->m_pScript);

            CBaseScript* pInternScript = static_cast<CBaseScript*>(pScriptComponent->m_pScript);

            pInternScript->m_IsActive = pScriptComponent->IsActiveAndUsable();
        }
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