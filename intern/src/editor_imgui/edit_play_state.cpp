
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_play_state.h"
#include "editor_imgui/edit_unload_map_state.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include "engine/camera/cam_control_manager.h"

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
} // namespace Edit