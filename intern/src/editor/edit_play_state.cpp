
#include "editor/edit_precompiled.h"

#include "camera/cam_control_manager.h"

#include "core/core_console.h"

#include "editor/edit_play_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CPlayState::CPlayState()
        : m_Action(CState::Play)
    {
        // -----------------------------------------------------------------------------
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::App_Exit, EDIT_RECEIVE_MESSAGE(&CPlayState::OnExit));
        Edit::MessageManager::Register(Edit::SGUIMessageType::App_Edit, EDIT_RECEIVE_MESSAGE(&CPlayState::OnEdit));
    }
    
    // -----------------------------------------------------------------------------
    
    CPlayState::~CPlayState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Activate game control for play state
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::GameControl);

        // Cam::ControlManager::GetActiveControl().SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
        
        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnLeave()
    {
        // -----------------------------------------------------------------------------
        // Reset action
        // -----------------------------------------------------------------------------
        m_Action = CState::Play;

        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnRun()
    {
        CState::EStateType NextState = CState::Play;

        switch (m_Action)
        {
        case Edit::CState::Exit:
            CUnloadMapState::GetInstance().SetNextState(CState::Exit);
            NextState = CState::UnloadMap;
            break;
        case Edit::CState::Edit:
            NextState = CState::Edit;
            break;
        }

        return NextState;
    }

    // -----------------------------------------------------------------------------

    void CPlayState::OnExit(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        m_Action = CState::Exit;
    }

    // -----------------------------------------------------------------------------

    void CPlayState::OnEdit(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        m_Action = CState::Edit;
    }
} // namespace Edit